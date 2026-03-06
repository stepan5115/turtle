#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include<map>
#include "settings.h"
#include "operations.h"
#include "render.h"

#include "generated/TurtleGrammarLexer.h"
#include "generated/TurtleGrammarParser.h"
#include "generated/TurtleGrammarBaseVisitor.h"

using namespace antlr4;

class ErrorCountingVisitor : public TurtleGrammarBaseVisitor {
private:
    int errorCount = 0;
public:
    std::any visitErrorNode(tree::ErrorNode *node) override {
        errorCount++;
        std::cerr << "Ошибка синтаксиса: " << node->getText() << std::endl;
        return {};
    }
    
    int getErrorCount() const { return errorCount; }
};
class TurtleVisitor : public TurtleGrammarBaseVisitor {
private:
    std::vector<std::unique_ptr<Operation>>& program;
    std::vector<std::map<std::string, int>> scopes;
    int getVariable(const std::string& name) {
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            auto var = it->find(name);
            if (var != it->end()) return var->second;
        }
        throw std::runtime_error("Переменная не найдена: " + name);
    }
    void setVariable(const std::string& name, int value) {
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            if (it->find(name) != it->end()) {
                it->at(name) = value;
                return;
            }
        }
        scopes.back()[name] = value;
    }
    int toInt(const std::string& s) {
        return std::stoi(s);
    }
    int evalExpression(TurtleGrammarParser::ExpressionContext* ctx) {
        if (auto num = dynamic_cast<TurtleGrammarParser::NumberExprContext*>(ctx)) {
            return std::stoi(num->NUMBER()->getText());
        }
        if (auto var = dynamic_cast<TurtleGrammarParser::VariableExprContext*>(ctx)) {
            return getVariable(var->IDENTIFIER()->getText());
        }
        if (auto paren = dynamic_cast<TurtleGrammarParser::ParenExprContext*>(ctx)) {
            return evalExpression(paren->expression());
        }
        if (auto unary = dynamic_cast<TurtleGrammarParser::UnaryMinusExprContext*>(ctx)) {
            return -evalExpression(unary->expression());
        }
        if (auto notExpr = dynamic_cast<TurtleGrammarParser::NotExprContext*>(ctx)) {
            return !evalExpression(notExpr->expression());
        }
        if (auto mulDiv = dynamic_cast<TurtleGrammarParser::MulDivExprContext*>(ctx)) {
            int left = evalExpression(mulDiv->expression(0));
            int right = evalExpression(mulDiv->expression(1));
            std::string op = mulDiv->op->getText();
            if (op == "*") return left * right;
            if (op == "/") {
                if (right == 0) throw std::runtime_error("Деление на ноль");
                return left / right;
            }
        }
        if (auto addSub = dynamic_cast<TurtleGrammarParser::AddSubExprContext*>(ctx)) {
            int left = evalExpression(addSub->expression(0));
            int right = evalExpression(addSub->expression(1));
            std::string op = addSub->op->getText();
            if (op == "+") return left + right;
            if (op == "-") return left - right;
        }
        if (auto rel = dynamic_cast<TurtleGrammarParser::RelationalExprContext*>(ctx)) {
            int left = evalExpression(rel->expression(0));
            int right = evalExpression(rel->expression(1));
            std::string op = rel->op->getText();
            if (op == "<") return left < right;
            if (op == ">") return left > right;
            if (op == "<=") return left <= right;
            if (op == ">=") return left >= right;
        }
        if (auto eq = dynamic_cast<TurtleGrammarParser::EqualityExprContext*>(ctx)) {
            int left = evalExpression(eq->expression(0));
            int right = evalExpression(eq->expression(1));
            std::string op = eq->op->getText();
            if (op == "==") return left == right;
            if (op == "!=") return left != right;
        }
        if (auto andExpr = dynamic_cast<TurtleGrammarParser::AndExprContext*>(ctx)) {
            if (!evalExpression(andExpr->expression(0))) return 0;
            return evalExpression(andExpr->expression(1)) != 0;
        }
        if (auto orExpr = dynamic_cast<TurtleGrammarParser::OrExprContext*>(ctx)) {
            if (evalExpression(orExpr->expression(0))) return 1;
            return evalExpression(orExpr->expression(1)) != 0;
        }
        throw std::runtime_error("Неизвестный тип выражения");
    }
public:
    TurtleVisitor(std::vector<std::unique_ptr<Operation>>& prog) : program(prog) {
        scopes.push_back({});
    }
    std::any visitResize(TurtleGrammarParser::ResizeContext* ctx) override {
        auto exprs = ctx->expression();
        int newWidth = evalExpression(exprs[0]);
        int newHeight = evalExpression(exprs[1]);
        
        program.push_back(std::make_unique<ResizeOperation>(newWidth, newHeight));
        return {};
    }
    std::any visitAssignmentStmt(TurtleGrammarParser::AssignmentStmtContext* ctx) override {
        std::string varName = ctx->assignment()->IDENTIFIER()->getText();
        int value = evalExpression(ctx->assignment()->expression());
        setVariable(varName, value);
        return {};
    }
    std::any visitIfStmt(TurtleGrammarParser::IfStmtContext* ctx) override {
        auto ifCtx = ctx->ifStatement();
        int cond = evalExpression(ifCtx->expression());
        
        if (cond) {
            visit(ifCtx->statement(0));
        } else if (ifCtx->statement().size() > 1) {
            visit(ifCtx->statement(1));
        }
        return {};
    }
    std::any visitWhileStmt(TurtleGrammarParser::WhileStmtContext* ctx) override {
        auto whileCtx = ctx->whileStatement();
        while (evalExpression(whileCtx->expression())) {
            visit(whileCtx->statement());
        }
        return {};
    }
    std::any visitForStmt(TurtleGrammarParser::ForStmtContext* ctx) override {
        auto forCtx = ctx->forStatement();
        scopes.push_back({});
        
        if (forCtx->assignment().size() > 0 && forCtx->assignment(0)) {
            std::string varName = forCtx->assignment(0)->IDENTIFIER()->getText();
            int value = evalExpression(forCtx->assignment(0)->expression());
            scopes.back()[varName] = value;
        }
        
        while (true) {
            if (forCtx->expression()) {
                if (!evalExpression(forCtx->expression())) break;
            }
            
            visit(forCtx->statement());
            
            if (forCtx->assignment().size() > 1 && forCtx->assignment(1)) {
                std::string varName = forCtx->assignment(1)->IDENTIFIER()->getText();
                int value = evalExpression(forCtx->assignment(1)->expression());
                setVariable(varName, value);
            }
        }
        
        scopes.pop_back();
        return {};
    }
    std::any visitBlockStmt(TurtleGrammarParser::BlockStmtContext* ctx) override {
        scopes.push_back({});
        for (auto stmt : ctx->block()->statement()) {
            visit(stmt);
        }
        scopes.pop_back();
        return {};
    }
    std::any visitPenUpCommand(TurtleGrammarParser::PenUpCommandContext* ctx) override {
        program.push_back(std::make_unique<SetDrawModeOperation>(false));
        return {};
    }
    std::any visitPenDownCommand(TurtleGrammarParser::PenDownCommandContext* ctx) override {
        program.push_back(std::make_unique<SetDrawModeOperation>(true));
        return {};
    }
    std::any visitMoveCommand(TurtleGrammarParser::MoveCommandContext* ctx) override {
        int distance = evalExpression(ctx->expression());
        program.push_back(std::make_unique<MoveOperation>(distance));
        return {};
    }
    std::any visitMoveToCommand(TurtleGrammarParser::MoveToCommandContext* ctx) override {
        auto exprs = ctx->expression();
        int x = evalExpression(exprs[0]);
        int y = evalExpression(exprs[1]);
        program.push_back(std::make_unique<MoveToOperation>(x, y));
        return {};
    }
    std::any visitSetColorCommand(TurtleGrammarParser::SetColorCommandContext* ctx) override {
        auto exprs = ctx->expression();
        RGB color(
            evalExpression(exprs[0]),
            evalExpression(exprs[1]),
            evalExpression(exprs[2])
        );
        program.push_back(std::make_unique<SetDrawColorOperation>(color));
        return {};
    }
    std::any visitFillFieldCommand(TurtleGrammarParser::FillFieldCommandContext* ctx) override {
        auto exprs = ctx->expression();
        RGB color(
            evalExpression(exprs[0]),
            evalExpression(exprs[1]),
            evalExpression(exprs[2])
        );
        program.push_back(std::make_unique<FillFieldOperation>(color));
        return {};
    }
    std::any visitSetDrawModeCommand(TurtleGrammarParser::SetDrawModeCommandContext* ctx) override {
        std::string mode = ctx->boolean()->getText();
        if (mode == "true") {
            program.push_back(std::make_unique<SetDrawModeOperation>(true));
        } else {
            program.push_back(std::make_unique<SetDrawModeOperation>(false));
        }
        return {};
    }
    std::any visitOrientationCommand(TurtleGrammarParser::OrientationCommandContext* ctx) override {
        std::string dir = ctx->direction()->getText();
        
        if (dir == "right") {
            program.push_back(std::make_unique<SetOrientationOperation>(RIGHT));
        } else if (dir == "left") {
            program.push_back(std::make_unique<SetOrientationOperation>(LEFT));
        } else if (dir == "up") {
            program.push_back(std::make_unique<SetOrientationOperation>(TOP));
        } else if (dir == "down") {
            program.push_back(std::make_unique<SetOrientationOperation>(BOTTOM));
        }
        
        return {};
    }
};

int main(int argc, char** argv) {
    try {
        if (argc != 2) {
            std::cerr << "Использование: " << argv[0] << " <файл_с_программой>" << std::endl;
            return 1;
        }
        std::string filename = argv[1];
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Ошибка: не удалось открыть файл " << filename << std::endl;
            return 1;
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string input = buffer.str();
        
        std::cout << "Файл загружен: " << filename << std::endl;
        std::cout << "Размер: " << input.length() << " байт" << std::endl;

        ANTLRInputStream inputStream(input);
        TurtleGrammarLexer lexer(&inputStream);
        CommonTokenStream tokens(&lexer);
        tokens.fill();
        std::cout << "\nНайдено токенов: " << tokens.size() << std::endl;
        std::cout << "Токены:" << std::endl;
        for (auto token : tokens.getTokens()) {
            std::cout << "  " << token->toString() << std::endl;
        }
        TurtleGrammarParser parser(&tokens);
        parser.setErrorHandler(std::make_shared<DefaultErrorStrategy>());
        std::cout << "\nРазбор программы..." << std::endl;
        tree::ParseTree* tree = parser.program();
        ErrorCountingVisitor errorVisitor;
        errorVisitor.visit(tree);
        std::cout << "\nРезультат:" << std::endl;
        std::cout << "  Ошибок: " << errorVisitor.getErrorCount() << std::endl;
        
        if (errorVisitor.getErrorCount() == 0) {
            std::cout << "  Программа синтаксически корректна! Выполняем..." << std::endl;
            std::vector<std::unique_ptr<Operation>> program;
            TurtleVisitor visitor(program);
            visitor.visit(tree);
            Settings settings(30, 30);
            settings.fillField(RGB(0, 0, 0));
            Render::run(settings, program, 300, 800, 600, argc, argv);
        } else {
            std::cout << "  Программа содержит ошибки." << std::endl;
        }
        /*
        Settings settings(30, 30);
        settings.fillField(RGB(0, 0, 0));
        
        std::vector<std::unique_ptr<Operation>> program;
        
        program.push_back(std::make_unique<FillFieldOperation>(RGB(0, 0, 0)));
        // Рисуем спираль
        int x = 15, y = 15;  // Центр
        int step = 1;
        int direction = 0;  // 0-вниз, 1-вправо, 2-вверх, 3-влево
        program.push_back(std::make_unique<MoveToOperation>(x, y));
        program.push_back(std::make_unique<SetDrawModeOperation>(true));
        program.push_back(std::make_unique<SetDrawColorOperation>(RGB(255, 0, 0)));
        for (int i = 0; i < 15; i++) {
            for (int j = 0; j < 2; j++) {
                // Меняем цвет
                program.push_back(std::make_unique<SetDrawColorOperation>(
                    RGB(rand() % 256, rand() % 256, rand() % 256)));
                // Двигаемся в текущем направлении
                switch (direction) {
                    case 0: // вниз
                        program.push_back(std::make_unique<SetOrientationOperation>(BOTTOM));
                        program.push_back(std::make_unique<MoveOperation>(step));
                        break;
                    case 1: // вправо
                        program.push_back(std::make_unique<SetOrientationOperation>(RIGHT));
                        program.push_back(std::make_unique<MoveOperation>(step));
                        break;
                    case 2: // вверх
                        program.push_back(std::make_unique<SetOrientationOperation>(TOP));
                        program.push_back(std::make_unique<MoveOperation>(step));
                        break;
                    case 3: // влево
                        program.push_back(std::make_unique<SetOrientationOperation>(LEFT));
                        program.push_back(std::make_unique<MoveOperation>(step));
                        break;
                }
                direction = (direction + 1) % 4;
            }
            step++;
        }

        program.push_back(std::make_unique<SetDrawModeOperation>(false));
        Render::run(settings, program, 300, 800, 600, argc, argv);
        */
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}