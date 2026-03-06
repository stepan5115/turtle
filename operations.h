#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "settings.h"

class Operation {
public:
    virtual void execute(Settings& settings) = 0; 
};

class MoveOperation : public Operation {
private:
    unsigned int dest;
public:
    MoveOperation(unsigned int dest) {
        this->dest = dest;
    }
    void execute(Settings& settings) override {
        settings.move(dest);
    }
};

class MoveToOperation : public Operation {
private:
    unsigned int x;
    unsigned int y;
public:
    MoveToOperation(unsigned int x, unsigned int y) {
        this->x = x;
        this->y=y;
    }
    void execute(Settings& settings) override {
        settings.moveTo(x, y);
    }
};

class FillFieldOperation : public Operation {
private:
    RGB color;
public:
    FillFieldOperation(RGB color) {
        this->color = color;
    }
    void execute(Settings& settings) override {
        settings.fillField(color);
    }
};

class SetDrawColorOperation : public Operation {
private:
    RGB color;
public:
    SetDrawColorOperation(RGB color) {
        this->color = color;
    }
    void execute(Settings& settings) override {
        settings.setDrawColor(color);
    }
};

class SetDrawModeOperation : public Operation {
private:
    bool mode;
public:
    SetDrawModeOperation(bool mode) {
        this->mode = mode;
    }
    void execute(Settings& settings) override {
        settings.setDrawMode(mode);
    }
};

class SetOrientationOperation : public Operation {
private:
    Orientation orientation;
public:
    SetOrientationOperation(Orientation orientation) {
        this->orientation = orientation;
    }
    void execute(Settings& settings) override {
        settings.setOrientation(orientation);
    }
};

class ResizeOperation : public Operation {
private:
    int newWidth;
    int newHeight;
public:
    ResizeOperation(int w, int h) : newWidth(w), newHeight(h) {}
    
    void execute(Settings& settings) override {
        settings.resize(newWidth, newHeight);
    }
};

#endif