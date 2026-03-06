#ifndef SETTINGS_H
#define SETTINGS_H

#include <algorithm>
#include<cmath>
#include <stdexcept>
#include<vector>

enum Orientation {
    TOP,
    BOTTOM,
    RIGHT,
    LEFT
};

struct RGB {
    unsigned char r, g, b;
    RGB() : r(0), g(0), b(0) {}
    RGB(unsigned char red, unsigned char green, unsigned char blue) 
        : r(red), g(green), b(blue) {}
};

class Settings {
private:
    bool isDrawModOn = false;
    RGB drawColor = RGB();
    Orientation orientation = BOTTOM;
    unsigned int xCoord = 0; //0-лево, чем больше, тем правее
    unsigned int yCoord = 0; //0-верх, чем больше, тем ниже
    //настройки высоты и ширины поля
    unsigned int width;
    unsigned int height;
    //Само поле
    std::vector<std::vector<RGB>> field;
    void initField() {
        field = std::vector<std::vector<RGB>>(height, std::vector<RGB>(width));
    }
public:
    Settings(unsigned int width, unsigned int height) {
        if (width == 0 || height == 0)
            throw std::runtime_error("поле не может быть размера 0");
        this->width = width;
        this->height = height;
        initField();
    }
    unsigned int getWidth() const {
        return width;
    }
    unsigned int getHeight() const {
        return height;
    }
    const auto& getField() const {
        return field;
    }
    unsigned int getX() const {
        return xCoord;
    }
    unsigned int getY() const {
        return yCoord;
    }
    void fillField(RGB color) {
        for (unsigned  int i = 0; i < height; i++)
            for (unsigned int j = 0; j < width; j++)
                field[i][j] = color;
    }
    void setDrawMode(bool isDrawModOn) {
        this->isDrawModOn = isDrawModOn;
    }
    void setDrawColor(RGB color) {
        this->drawColor = color;
    }
    void setOrientation(Orientation orientation) {
        this->orientation = orientation;
    }
    void move(unsigned int dest) {
        unsigned int oldX = xCoord;
        unsigned int oldY = yCoord;
        switch (orientation) {
            case TOP:
                yCoord = (yCoord > dest) ? (yCoord - dest) : 0;
                break;
            case BOTTOM:
                yCoord = std::min(yCoord + dest, height - 1);
                break;
            case RIGHT:
                xCoord = std::min(xCoord + dest, width - 1);
                break;
            case LEFT:
                xCoord = (xCoord > dest) ? (xCoord - dest) : 0;
                break;
        }
        if (!isDrawModOn)
            return;
        if (orientation == RIGHT || orientation == LEFT) {
            unsigned int startX = std::min(oldX, xCoord);
            unsigned int endX = std::max(oldX, xCoord);
            for (unsigned int x = startX; x <= endX; x++)
                field[oldY][x] = drawColor;
        } else {
            unsigned int startY = std::min(oldY, yCoord);
            unsigned int endY = std::max(oldY, yCoord);
            for (unsigned int y = startY; y <= endY; y++)
                field[y][oldX] = drawColor;
        }
    }
    void drawLine(unsigned int x1, unsigned int y1, 
              unsigned int x2, unsigned int y2) {
        int x = x1;
        int y = y1;
        int dx = abs(static_cast<int>(x2) - static_cast<int>(x1));
        int dy = abs(static_cast<int>(y2) - static_cast<int>(y1));
        int sx = (x1 < x2) ? 1 : -1;
        int sy = (y1 < y2) ? 1 : -1;
        int err = dx - dy;
        
        while (true) {
            if (x >= 0 && x < static_cast<int>(width) && 
                y >= 0 && y < static_cast<int>(height)) {
                field[y][x] = drawColor;
            }
            
            if (x == static_cast<int>(x2) && y == static_cast<int>(y2)) break;
            
            int e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x += sx;
            }
            if (e2 < dx) {
                err += dx;
                y += sy;
            }
        }
    }

    void moveTo(unsigned int newX, unsigned int newY) {
        if (newX >= width || newY >= height) {
            throw std::out_of_range("Координаты вне поля");
        }
        
        unsigned int oldX = xCoord;
        unsigned int oldY = yCoord;
        
        xCoord = newX;
        yCoord = newY;
        
        if (isDrawModOn) {
            drawLine(oldX, oldY, newX, newY);
        }
    }
};

#endif