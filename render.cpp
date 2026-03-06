#include "render.h"
#include <iostream>

Settings* Render::currentSettings = nullptr;
std::vector<std::unique_ptr<Operation>>* Render::currentProgram = nullptr;
size_t Render::currentStep = 0;
int Render::delayMs = 500;
int Render::windowWidth = 800;
int Render::windowHeight = 600;

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
        glRasterPos2i(10, windowHeight - 20);
        std::string info = "Step: " + std::to_string(currentStep) + "/" + 
                          std::to_string(currentProgram ? currentProgram->size() : 0);
        for (char c : info) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
        }
    }
    
    glutSwapBuffers();
}

void Render::keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 27: // ESC
            exit(0);
            break;
        case ' ': // Space - следующий шаг
            executeNextStep();
            break;
        case 'a': // A - выполнить все
        case 'A':
            while (currentStep < currentProgram->size()) {
                executeNextStep();
            }
            break;
    }
}

void Render::timer(int value) {
    if (currentStep < currentProgram->size()) {
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
              << "Текущая задержка: " << delayMs << "ms\n";
    
    glutMainLoop();
}