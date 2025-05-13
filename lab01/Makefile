BUILD ?= debug

# Каталоги
SRC_DIR := ./src
INC_DIR := ./include
LIB_DIR := ./lib
OUT_DIR := ./out/$(BUILD)

# Имя бинарного файла
TARGET := $(OUT_DIR)/dirwalk

# Исходные файлы
SRCS := $(SRC_DIR)/main.c $(SRC_DIR)/options.c $(SRC_DIR)/dirwalk.c

# Компилятор
CC := gcc

# Общие флаги компиляции: добавляем каталоги с заголовочными файлами
COMMON_CFLAGS := -D_GNU_SOURCE -D_POSIX_C_SOURCE=200112L -W -Wall -Wextra -std=c11 -pedantic -Wno-unused-parameter -Wno-unused-variable -I$(INC_DIR) -I$(SRC_DIR)

# Флаги зависят от типа сборки (debug или release)
ifeq ($(BUILD),debug)
	CFLAGS := $(COMMON_CFLAGS) -g -O0
else ifeq ($(BUILD),release)
	CFLAGS := $(COMMON_CFLAGS) -O2
else
	$(error Unknown build type: $(BUILD))
endif

# Если нужны дополнительные библиотеки, указываем путь к ним
LDFLAGS := -L$(LIB_DIR)

.PHONY: all clean debug release

# Сборка бинарного файла из исходников напрямую (без .o файлов)
all: $(TARGET)

$(TARGET):
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

debug:
	$(MAKE) BUILD=debug all

release:
	$(MAKE) BUILD=release all

clean:
	rm -rf ./out
