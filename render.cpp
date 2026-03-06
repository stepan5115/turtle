#include "render.h"
#include <iostream>
#include <sstream>
#include <string>

Settings* Render::currentSettings = nullptr;
std::vector<std::unique_ptr<Operation>>* Render::currentProgram = nullptr;
size_t Render::currentStep = 0;
int Render::delayMs = 500;
int Render::windowWidth = 800;
int Render::windowHeight = 600;
bool Render::pausedTimer = false;
Settings Render::initialSettings;

void drawText(float x, float y, const std::string& text, void* font = GLUT_BITMAP_HELVETICA_12) {
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(font, c);
    }
}

std::string Render::getOperationName(size_t step) {
    if (!currentProgram || step >= currentProgram->size()) return "None";
    const auto& op = (*currentProgram)[step];
    
    if (dynamic_cast<MoveOperation*>(op.get())) return "MOVE";
    if (dynamic_cast<MoveToOperation*>(op.get())) return "MOVE_TO";
    if (dynamic_cast<SetDrawColorOperation*>(op.get())) return "COLOR";
    if (dynamic_cast<SetDrawModeOperation*>(op.get())) {
        return "DRAW_MODE";
    }
    if (dynamic_cast<SetOrientationOperation*>(op.get())) {
        return "ORIENT";
    }
    if (dynamic_cast<FillFieldOperation*>(op.get())) return "FILL";
    return "UNKNOWN";
}

std::string Render::getOrientationString() {
    if (!currentSettings) return "?";
    switch (currentSettings->getOrientation()) {
        case TOP: return "UP";
        case BOTTOM: return "DOWN";
        case RIGHT: return "RIGHT";
        case LEFT: return "LEFT";
        default: return "?";
    }
}

void Render::reset() {
    if (currentSettings) {
        *currentSettings = initialSettings;  // Восстанавливаем начальные настройки
        currentStep = 0;
        glutPostRedisplay();
        std::cout << "Сброс выполнен" << std::endl;
    }
}

void Render::display() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    if (currentSettings) {
        const auto& field = currentSettings->getField();
        int height = currentSettings->getHeight();
        int width = currentSettings->getWidth();
        
        float cellWidth = static_cast<float>(windowWidth) / width;
        float cellHeight = static_cast<float>(windowHeight) / height;
        
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                RGB color = field[y][x];
                glColor3ub(color.r, color.g, color.b);
                
                glBegin(GL_QUADS);
                glVertex2f(x * cellWidth, (height - y - 1) * cellHeight);
                glVertex2f((x + 1) * cellWidth, (height - y - 1) * cellHeight);
                glVertex2f((x + 1) * cellWidth, (height - y) * cellHeight);
                glVertex2f(x * cellWidth, (height - y) * cellHeight);
                glEnd();
            }
        }
        
        glColor3ub(100, 100, 100);
        glBegin(GL_LINES);
        for (int i = 0; i <= width; i++) {
            glVertex2f(i * cellWidth, 0);
            glVertex2f(i * cellWidth, windowHeight);
        }
        for (int i = 0; i <= height; i++) {
            glVertex2f(0, i * cellHeight);
            glVertex2f(windowWidth, i * cellHeight);
        }
        glEnd();
        
        float turtleX = currentSettings->getX() * cellWidth;
        float turtleY = (height - currentSettings->getY() - 1) * cellHeight;
        
        glColor3ub(255, 255, 0);
        glBegin(GL_QUADS);
        glVertex2f(turtleX, turtleY);
        glVertex2f(turtleX + cellWidth, turtleY);
        glVertex2f(turtleX + cellWidth, turtleY + cellHeight);
        glVertex2f(turtleX, turtleY + cellHeight);
        glEnd();
        
        glColor3ub(255, 255, 255);
        float arrowSize = cellWidth * 0.3f;
        float centerX = turtleX + cellWidth/2;
        float centerY = turtleY + cellHeight/2;
        
        glBegin(GL_LINES);
        switch (currentSettings->getOrientation()) {
            case TOP:
                glVertex2f(centerX, centerY - arrowSize);
                glVertex2f(centerX, centerY + arrowSize);
                glVertex2f(centerX, centerY + arrowSize);
                glVertex2f(centerX - arrowSize/2, centerY + arrowSize/2);
                glVertex2f(centerX, centerY + arrowSize);
                glVertex2f(centerX + arrowSize/2, centerY + arrowSize/2);
                break;
            case BOTTOM:
                glVertex2f(centerX, centerY + arrowSize);
                glVertex2f(centerX, centerY - arrowSize);
                glVertex2f(centerX, centerY - arrowSize);
                glVertex2f(centerX - arrowSize/2, centerY - arrowSize/2);
                glVertex2f(centerX, centerY - arrowSize);
                glVertex2f(centerX + arrowSize/2, centerY - arrowSize/2);
                break;
            case RIGHT:
                glVertex2f(centerX - arrowSize, centerY);
                glVertex2f(centerX + arrowSize, centerY);
                glVertex2f(centerX + arrowSize, centerY);
                glVertex2f(centerX + arrowSize/2, centerY - arrowSize/2);
                glVertex2f(centerX + arrowSize, centerY);
                glVertex2f(centerX + arrowSize/2, centerY + arrowSize/2);
                break;
            case LEFT:
                glVertex2f(centerX + arrowSize, centerY);
                glVertex2f(centerX - arrowSize, centerY);
                glVertex2f(centerX - arrowSize, centerY);
                glVertex2f(centerX - arrowSize/2, centerY - arrowSize/2);
                glVertex2f(centerX - arrowSize, centerY);
                glVertex2f(centerX - arrowSize/2, centerY + arrowSize/2);
                break;
        }
        glEnd();
        
        // === ОТОБРАЖЕНИЕ ИНФОРМАЦИИ ===
        glColor3ub(255, 255, 255);
        
        drawText(10, windowHeight - 20, "Step: " + std::to_string(currentStep) + "/" + 
                 std::to_string(currentProgram ? currentProgram->size() : 0));
        
        std::string pauseStatus = pausedTimer ? "PAUSED" : "RUNNING";
        glColor3ub(pausedTimer ? 255 : 0, pausedTimer ? 0 : 255, 0);
        std::string pauseText = "Timer: " + pauseStatus;
        drawText(windowWidth - 150, windowHeight - 20, pauseText);
        
        glColor3ub(200, 200, 200);
        std::stringstream posStream;
        posStream << "Pos: (" << currentSettings->getX() << ", " << currentSettings->getY() << ")";
        drawText(10, 20, posStream.str());
        
        std::string dirText = "Dir: " + getOrientationString();
        drawText(10, 40, dirText);
        
        if (currentStep < currentProgram->size()) {
            glColor3ub(100, 255, 100);
            std::string opText = "Next: " + getOperationName(currentStep);
            drawText(windowWidth - 150, 40, opText);
        }
        
        glColor3ub(150, 150, 150);
        std::string helpText = "ESC:exit SPACE:step A:all P:pause";
        float helpWidth = helpText.length() * 8;
        drawText((windowWidth - helpWidth)/2, 10, helpText);
    }
    
    glutSwapBuffers();
}

void Render::keyboard(unsigned char key, int /*x*/, int /*y*/) {
    switch (key) {
        case 27: // ESC
            exit(0);
            break;
        case ' ': // Space - следующий шаг
            executeNextStep();
            break;
        case 'p':
        case 'P':
            pausedTimer = !pausedTimer;
            std::cout << (pausedTimer ? "Пауза" : "Продолжение") << std::endl;
            glutPostRedisplay();
            break;
        case 'a': // A - выполнить все
        case 'A':
            while (currentStep < currentProgram->size()) {
                executeNextStep();
            }
            break;
        case 'r': // R - сброс
        case 'R':
            reset();
            break;
    }
}

void Render::timer(int /*value*/) {
    if (currentStep < currentProgram->size()) {
        if (!pausedTimer)
            executeNextStep();
        glutTimerFunc(delayMs, timer, 0);
    }
}

void Render::executeNextStep() {
    if (currentStep < currentProgram->size()) {
        (*currentProgram)[currentStep]->execute(*currentSettings);
        currentStep++;
        glutPostRedisplay();
    }
}

void Render::run(Settings& settings, 
                std::vector<std::unique_ptr<Operation>>& program,
                int delay,
                int winWidth,
                int winHeight,
                int argc,
                char** argv) {

    initialSettings = settings;
    currentSettings = &settings;
    currentProgram = &program;
    currentStep = 0;
    delayMs = delay;
    windowWidth = winWidth;
    windowHeight = winHeight;
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Черепашка - Анимация");
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glMatrixMode(GL_MODELVIEW);
    
    glClearColor(0, 0, 0, 1);
    
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(delayMs, timer, 0);
    
    std::cout << "Управление:\n"
              << "  ESC - выход\n"
              << "  SPACE - следующий шаг\n"
              << "  A - выполнить все\n"
              << "  R - сброс\n"
              << "  P - (поставить на паузу)/(возобновить) автоматическое выполнение"
              << "Текущая задержка: " << delayMs << "ms\n";
    
    glutMainLoop();
}