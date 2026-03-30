# ============================================
# Makefile для проекта с ANTLR4 и OpenGL
# Использование: make GRAMMAR_FILE=путь/к/грамматике.g4
# ============================================

# Пути и версии
ANTLR_VERSION = 4.13.2
ANTLR_JAR = antlr4-$(ANTLR_VERSION)-complete.jar
ANTLR_DIR = antlr
ANTLR_URL = https://repo1.maven.org/maven2/org/antlr/antlr4/$(ANTLR_VERSION)/$(ANTLR_JAR)

JRE_DIR = jre
JAVA = $(JRE_DIR)/bin/java
ANTLR_RUNTIME_DIR = $(PWD)/antlr4-runtime
GENERATED_DIR = generated

# Конфигурация компилятора
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -g -I. -Igenerated -I$(ANTLR_RUNTIME_DIR)/include/antlr4-runtime -Wno-overloaded-virtual -Wno-unused-parameter
LDFLAGS = -L$(ANTLR_RUNTIME_DIR)/lib -lGL -lGLU -lglut -lantlr4-runtime
TARGET = turtle
SOURCES = main.cpp render.cpp
HEADERS = settings.h operations.h render.h
GENERATED_FILES = $(GENERATED_DIR)/TurtleGrammarLexer.cpp \
                  $(GENERATED_DIR)/TurtleGrammarParser.cpp \
                  $(GENERATED_DIR)/TurtleGrammarBaseVisitor.cpp \
                  $(GENERATED_DIR)/TurtleGrammarVisitor.cpp
GRAMMAR_FILE ?= grammars/TurtleGrammar.g4

# ============================================
# Генерация ANTLR
# ============================================
$(GENERATED_FILES): $(GRAMMAR_FILE)
	@echo "Генерация парсера из $<..."
	@mkdir -p $(GENERATED_DIR)
	@$(JAVA) -jar $(ANTLR_DIR)/$(ANTLR_JAR) -Dlanguage=Cpp -visitor -no-listener -o $(GENERATED_DIR) $< -Xexact-output-dir
	@echo "Генерация завершена"

# ============================================
# Сборка исполняемого файла
# ============================================
$(TARGET): $(SOURCES) $(GENERATED_FILES)
	@echo "Компиляция проекта..."
	$(CXX) $(CXXFLAGS) $(SOURCES) $(GENERATED_DIR)/*.cpp -o $(TARGET) $(LDFLAGS)
	@echo "Сборка завершена: $(TARGET)"

all: setup_deps $(TARGET)

# ============================================
# Очистка
# ============================================
clean:
	rm -rf $(TARGET) *.o *~ *.gch $(GENERATED_DIR)

clean-all: clean
	rm -rf $(ANTLR_DIR) $(JRE_DIR) $(ANTLR_RUNTIME_DIR)

# ============================================
# Зависимости по желанию
# ============================================
.PHONY: setup_antlr_jar
setup_antlr_jar:
	@if [ ! -f "$(ANTLR_DIR)/$(ANTLR_JAR)" ]; then \
		echo "ANTLR jar не найден, скачиваем..."; \
		mkdir -p $(ANTLR_DIR); \
		if command -v wget > /dev/null; then \
			wget -O $(ANTLR_DIR)/$(ANTLR_JAR) $(ANTLR_URL); \
		elif command -v curl > /dev/null; then \
			curl -L -o $(ANTLR_DIR)/$(ANTLR_JAR) $(ANTLR_URL); \
		else \
			echo "Не найден wget или curl. Установите один из них."; \
			exit 1; \
		fi; \
		if [ ! -s "$(ANTLR_DIR)/$(ANTLR_JAR)" ]; then \
			echo "Ошибка: jar пустой или не скачался"; \
			rm -f $(ANTLR_DIR)/$(ANTLR_JAR); \
			exit 1; \
		fi; \
		echo "ANTLR jar готов"; \
	else \
		echo "ANTLR jar уже есть"; \
	fi

.PHONY: setup_jre
setup_jre:
	@if [ ! -x "$(JAVA)" ]; then \
		echo "JRE не найдена, скачиваем..."; \
		mkdir -p $(JRE_DIR); \
		wget -O $(JRE_DIR)/jre.tar.gz \
		https://github.com/adoptium/temurin11-binaries/releases/download/jdk-11.0.22%2B7/OpenJDK11U-jre_x64_linux_hotspot_11.0.22_7.tar.gz || exit 1; \
		tar -xzf $(JRE_DIR)/jre.tar.gz -C $(JRE_DIR) --strip-components=1; \
		rm $(JRE_DIR)/jre.tar.gz; \
		echo "JRE установлена"; \
	else \
		echo "JRE уже есть"; \
	fi

.PHONY: check_opengl
check_opengl:
	@echo "Проверка OpenGL..."
	@if pkg-config --exists gl glu glut 2>/dev/null; then \
		echo "OpenGL найден"; \
	else \
		echo "OpenGL не найден. Установите вручную: libgl1-mesa-dev libglu1-mesa-dev freeglut3-dev pkg-config"; \
	fi

.PHONY: setup_antlr_runtime
setup_antlr_runtime:
	@set -e; \
	if [ ! -f "$(ANTLR_RUNTIME_DIR)/include/antlr4-runtime/antlr4-runtime.h" ]; then \
		echo "ANTLR4 C++ runtime не найден, собираем..."; \
		rm -rf /tmp/antlr4; \
		git clone --depth 1 --branch $(ANTLR_VERSION) https://github.com/antlr/antlr4.git /tmp/antlr4; \
		cd /tmp/antlr4/runtime/Cpp; \
		mkdir -p build; cd build; \
		cmake -DCMAKE_INSTALL_PREFIX=$(ANTLR_RUNTIME_DIR) ..; \
		make -j$$(nproc); \
		make install; \
		rm -rf /tmp/antlr4; \
		rm -rf build; \
		echo "ANTLR4 C++ runtime установлен"; \
	else \
		echo "ANTLR4 C++ runtime уже установлен"; \
	fi

.PHONY: setup_deps
setup_deps: setup_antlr_jar setup_jre check_opengl setup_antlr_runtime
	@echo "Все зависимости готовы (по желанию)."

# ============================================
# Запуск
# ============================================
run: $(TARGET)
	LD_LIBRARY_PATH=$(ANTLR_RUNTIME_DIR)/lib ./$(TARGET) $(INPUT_FILE) $(FLAGS)

debug: $(TARGET)
	gdb ./$(TARGET)

valgrind: $(TARGET)
	valgrind --leak-check=full ./$(TARGET)

.PHONY: all clean generated download_antlr download_jre check_opengl setup_deps run debug valgrind