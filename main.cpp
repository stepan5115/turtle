#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
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
    int toInt(const std::string& s) {
        return std::stoi(s);
    }
public:
    TurtleVisitor(std::vector<std::unique_ptr<Operation>>& prog) 
        : program(prog) {}
    std::any visitPenUpCommand(TurtleGrammarParser::PenUpCommandContext* ctx) override {
        program.push_back(std::make_unique<SetDrawModeOperation>(false));
        return {};
    }
    std::any visitPenDownCommand(TurtleGrammarParser::PenDownCommandContext* ctx) override {
        program.push_back(std::make_unique<SetDrawModeOperation>(true));
        return {};
    }
    std::any visitMoveCommand(TurtleGrammarParser::MoveCommandContext* ctx) override {
        int distance = toInt(ctx->NUMBER()->getText());
        program.push_back(std::make_unique<MoveOperation>(distance));
        return {};
    }
    std::any visitMoveToCommand(TurtleGrammarParser::MoveToCommandContext* ctx) override {
        auto numbers = ctx->NUMBER();
        int x = toInt(numbers[0]->getText());
        int y = toInt(numbers[1]->getText());
        program.push_back(std::make_unique<MoveToOperation>(x, y));
        return {};
    }
    std::any visitSetColorCommand(TurtleGrammarParser::SetColorCommandContext* ctx) override {
        auto numbers = ctx->NUMBER();
        RGB color(
            toInt(numbers[0]->getText()),
            toInt(numbers[1]->getText()),
            toInt(numbers[2]->getText())
        );
        program.push_back(std::make_unique<SetDrawColorOperation>(color));
        return {};
    }
    std::any visitFillFieldCommand(TurtleGrammarParser::FillFieldCommandContext* ctx) override {
        auto numbers = ctx->NUMBER();
        RGB color(
            toInt(numbers[0]->getText()),
            toInt(numbers[1]->getText()),
            toInt(numbers[2]->getText())
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