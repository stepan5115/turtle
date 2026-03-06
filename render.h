#ifndef RENDER_H
#define RENDER_H

#include "settings.h"
#include "operations.h"
#include <vector>
#include <memory>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

class Render {
private:
    static Settings* currentSettings;
    static Settings initialSettings;
    static std::vector<std::unique_ptr<Operation>>* currentProgram;
    static size_t currentStep;
    static int delayMs;
    static int windowWidth;
    static int windowHeight;
    static bool pausedTimer;
    
    Render() = delete;
    
    static void display();
    static void keyboard(unsigned char key, int x, int y);
    static void timer(int value);
    static void executeNextStep();
    static std::string getOperationName(size_t step);
    static std::string getOrientationString();
    static void reset();
public:
    static void run(Settings& settings, 
                   std::vector<std::unique_ptr<Operation>>& program,
                   int delay = 500,
                   int winWidth = 800, 
                   int winHeight = 600,
                   int argc = 0, 
                   char** argv = nullptr);
    
    static void setDelay(int ms) { delayMs = ms; }
    static void setPaused(bool paused) { pausedTimer = paused; }
    static bool isPaused() { return pausedTimer; }
};

#endif