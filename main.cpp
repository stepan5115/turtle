#include <iostream>
#include <vector>
#include <memory>
#include "settings.h"
#include "operations.h"
#include "render.h"

int main(int argc, char** argv) {
    try {
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
        
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}