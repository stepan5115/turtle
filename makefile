# ============================================
# Универсальный Makefile для проекта с ANTLR4 и OpenGL
# Использование: make GRAMMAR=путь/к/грамматике.g4
# ============================================

# Конфигурация
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++14 -g -I.
LDFLAGS = -lGL -lGLU -lglut
TARGET = turtle
SOURCES = main.cpp render.cpp
HEADERS = settings.h operations.h render.h
GRAMMAR_FILE ?= grammar.g4  # Можно переопределить при вызове: make GRAMMAR=mygrammar.g4
ANTLR_JAR = antlr-4.13.2-complete.jar
ANTLR_URL = https://www.antlr.org/download/$(ANTLR_JAR)
JRE_URL = https://github.com/adoptium/temurin11-binaries/releases/download/jdk-11.0.22%2B7/OpenJDK11U-jre_x64_linux_hotspot_11.0.22_7.tar.gz
JRE_DIR = jre
ANTLR_DIR = antlr

# Цвета для вывода
RED = \033[0;31m
GREEN = \033[0;32m
YELLOW = \033[1;33m
BLUE = \033[0;34m
NC = \033[0m

# Проверка наличия OpenGL
check_opengl:
	@echo "$(BLUE)Проверка наличия OpenGL...$(NC)"
	@if pkg-config --exists gl glu glut 2>/dev/null; then \
		echo "$(GREEN)  OpenGL найден$(NC)"; \
	else \
		echo "$(YELLOW)  OpenGL не найден в системе. Попытка установки...$(NC)"; \
		if command -v apt-get > /dev/null; then \
			sudo apt-get update && sudo apt-get install -y libgl1-mesa-dev libglu1-mesa-dev freeglut3-dev pkg-config; \
		elif command -v yum > /dev/null; then \
			sudo yum install -y mesa-libGL-devel mesa-libGLU-devel freeglut-devel pkgconfig; \
		elif command -v pacman > /dev/null; then \
			sudo pacman -S --noconfirm mesa glu freeglut pkg-config; \
		else \
			echo "$(RED)  Не удалось установить OpenGL. Менеджер пакетов не поддерживается.$(NC)"; \
			echo "$(RED)  Установите вручную: libgl1-mesa-dev libglu1-mesa-dev freeglut3-dev pkg-config$(NC)"; \
			exit 1; \
		fi; \
		echo "$(GREEN)  OpenGL успешно установлен$(NC)"; \
	fi
# Проверка наличия ANTLR
check_antlr: check_jre
	@echo "$(BLUE)Проверка наличия ANTLR4...$(NC)"
	@if [ -f "$(ANTLR_DIR)/$(ANTLR_JAR)" ]; then \
		echo "$(GREEN)  ANTLR4 найден$(NC)"; \
	else \
		echo "$(YELLOW)  ANTLR4 не найден. Скачивание...$(NC)"; \
		mkdir -p $(ANTLR_DIR); \
		if command -v wget > /dev/null; then \
			wget -O $(ANTLR_DIR)/$(ANTLR_JAR) $(ANTLR_URL); \
		elif command -v curl > /dev/null; then \
			curl -L -o $(ANTLR_DIR)/$(ANTLR_JAR) $(ANTLR_URL); \
		else \
			echo "$(RED)  Не найден wget или curl. Установите один из них.$(NC)"; \
			exit 1; \
		fi; \
		if [ -f "$(ANTLR_DIR)/$(ANTLR_JAR)" ]; then \
			echo "$(GREEN)  ANTLR4 успешно скачан$(NC)"; \
		else \
			echo "$(RED)  Не удалось скачать ANTLR4$(NC)"; \
			exit 1; \
		fi; \
	fi
# Проверка наличия JRE
check_jre:
	@echo "$(BLUE)Проверка наличия JRE...$(NC)"
	@if command -v java > /dev/null; then \
		JAVA_VERSION=$$(java -version 2>&1 | head -n 1 | cut -d'"' -f2 | cut -d'.' -f1); \
		if [ "$$JAVA_VERSION" -ge 11 ]; then \
			echo "$(GREEN)  JRE $$JAVA_VERSION найден$(NC)"; \
		else \
			echo "$(YELLOW)  JRE версии $$JAVA_VERSION устарел. Нужна версия 11 или выше.$(NC)"; \
			$(MAKE) download_jre; \
		fi; \
	else \
		echo "$(YELLOW)  JRE не найден. Скачивание...$(NC)"; \
		$(MAKE) download_jre; \
	fi
# Скачивание JRE
download_jre:
	@echo "$(BLUE)Скачивание JRE 11...$(NC)"
	@mkdir -p $(JRE_DIR)
	@if command -v wget > /dev/null; then \
		wget -O $(JRE_DIR)/jre.tar.gz $(JRE_URL); \
	elif command -v curl > /dev/null; then \
		curl -L -o $(JRE_DIR)/jre.tar.gz $(JRE_URL); \
	else \
		echo "$(RED)  Не найден wget или curl$(NC)"; \
		exit 1; \
	fi
	@tar -xzf $(JRE_DIR)/jre.tar.gz -C $(JRE_DIR) --strip-components=1
	@rm $(JRE_DIR)/jre.tar.gz
	@echo "$(GREEN)  JRE успешно скачана в $(JRE_DIR)$(NC)"
	@echo "$(YELLOW)  Используйте: $(JRE_DIR)/bin/java для запуска ANTLR$(NC)"

# ============================================
# Генерация кода из грамматики ANTLR
# ============================================
generate_parser: check_antlr
	@echo "$(BLUE)Генерация парсера из $(GRAMMAR_FILE)...$(NC)"
	@if [ ! -f "$(GRAMMAR_FILE)" ]; then \
		echo "$(RED)  Файл грамматики $(GRAMMAR_FILE) не найден!$(NC)"; \
		echo "$(YELLOW)  Укажите путь: make GRAMMAR=путь/к/файлу.g4$(NC)"; \
		exit 1; \
	fi
	@if [ -d "$(JRE_DIR)" ]; then \
		$(JRE_DIR)/bin/java -jar $(ANTLR_DIR)/$(ANTLR_JAR) -o generated -Dlanguage=Cpp -no-listener -visitor $(GRAMMAR_FILE); \
	else \
		java -jar $(ANTLR_DIR)/$(ANTLR_JAR) -o generated -Dlanguage=Cpp -no-listener -visitor $(GRAMMAR_FILE); \
	fi
	@if [ $$? -eq 0 ]; then \
		echo "$(GREEN)  Парсер успешно сгенерирован в папке generated/$(NC)"; \
		SOURCES += generated/*.cpp; \
		HEADERS += generated/*.h; \
		CXXFLAGS += -Igenerated; \
	else \
		echo "$(RED)  Ошибка при генерации парсера$(NC)"; \
		exit 1; \
	fi

# ============================================
# Сборка проекта
# ============================================
# Проверка всех зависимостей
check_all: check_opengl check_antlr

# Сборка с проверкой зависимостей
$(TARGET): check_all generate_parser $(SOURCES) $(HEADERS)
	@echo "$(BLUE)Компиляция проекта...$(NC)"
	@$(CXX) $(CXXFLAGS) $(SOURCES) generated/*.cpp -o $(TARGET) $(LDFLAGS)
	@echo "$(GREEN)Сборка завершена успешно!$(NC)"

# Цель по умолчанию
all: $(TARGET)

# ============================================
# Очистка
# ============================================
clean:
	@echo "$(BLUE)Очистка...$(NC)"
	rm -rf $(TARGET) *.o *~ *.gch generated $(ANTLR_DIR) $(JRE_DIR)
	@echo "$(GREEN)Очистка завершена$(NC)"

clean_deps:
	@echo "$(BLUE)Удаление загруженных зависимостей...$(NC)"
	rm -rf $(ANTLR_DIR) $(JRE_DIR)
	@echo "$(GREEN)Зависимости удалены$(NC)"

# ============================================
# Запуск
# ============================================
run: $(TARGET)
	@echo "$(BLUE)Запуск программы...$(NC)"
	./$(TARGET)

debug: $(TARGET)
	gdb ./$(TARGET)

valgrind: $(TARGET)
	valgrind --leak-check=full ./$(TARGET)

# ============================================
# Информация и помощь
# ============================================

info:
	@echo "$(BLUE)Информация о проекте:$(NC)"
	@echo "  Компилятор: $(CXX)"
	@echo "  Флаги: $(CXXFLAGS)"
	@echo "  Библиотеки: $(LDFLAGS)"
	@echo "  Исполняемый файл: $(TARGET)"
	@echo "  Исходники: $(SOURCES)"
	@echo "  Заголовки: $(HEADERS)"
	@echo "  Грамматика: $(GRAMMAR_FILE)"
	@if [ -d "$(JRE_DIR)" ]; then \
		echo "  JRE: локальная ($$($(JRE_DIR)/bin/java -version 2>&1 | head -n 1))"; \
	else \
		echo "  JRE: системная ($$(java -version 2>&1 | head -n 1))"; \
	fi
	@if [ -f "$(ANTLR_DIR)/$(ANTLR_JAR)" ]; then \
		echo "  ANTLR: локальный"; \
	else \
		echo "  ANTLR: не найден"; \
	fi

help:
	@echo "$(BLUE)Доступные цели:$(NC)"
	@echo "  all                    - собрать программу (по умолчанию)"
	@echo "  clean                  - удалить временные файлы"
	@echo "  clean_deps             - удалить загруженные зависимости (ANTLR, JRE)"
	@echo "  run                    - запустить программу"
	@echo "  debug                  - запустить в отладчике gdb"
	@echo "  valgrind               - проверить утечки памяти"
	@echo "  check_opengl           - проверить наличие OpenGL"
	@echo "  check_antlr            - проверить наличие ANTLR"
	@echo "  check_jre              - проверить наличие JRE"
	@echo "  generate_parser        - сгенерировать парсер из грамматики"
	@echo "  info                   - показать информацию о проекте"
	@echo "  help                   - показать эту справку"
	@echo ""
	@echo "$(BLUE)Параметры:$(NC)"
	@echo "  GRAMMAR=файл.g4        - путь к файлу грамматики ANTLR (обязательно)"
	@echo ""
	@echo "$(YELLOW)Примеры:$(NC)"
	@echo "  make GRAMMAR=grammar/turtle.g4"
	@echo "  make run GRAMMAR=mygrammar.g4"
	@echo "  make clean_deps"

# ============================================
# Объявление целей
# ============================================

.PHONY: all clean clean_deps run debug valgrind help info \
        check_opengl check_antlr check_jre download_jre generate_parser check_all