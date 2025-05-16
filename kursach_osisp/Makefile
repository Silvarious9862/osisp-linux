# Определение типа сборки (по умолчанию debug)
BUILD ?= debug

# Каталоги
SRC_DIR := ./src
LIB_DIR := ./include
OUT_DIR := ./out/$(BUILD)

# Целевой файл
TARGET := $(OUT_DIR)/bdupes

# Исходные файлы
SRCS := $(SRC_DIR)/main.c $(SRC_DIR)/args.c $(SRC_DIR)/result.c $(SRC_DIR)/selection.c $(SRC_DIR)/deletion.c \
		$(SRC_DIR)/filter_size.c $(SRC_DIR)/filter_mime.c $(SRC_DIR)/filter_hash.c $(SRC_DIR)/filter_cmp.c

# Компилятор
CC := gcc

# Общие флаги компиляции
COMMON_CFLAGS := -D_GNU_SOURCE -D_POSIX_C_SOURCE=200112L -W -Wall -Wextra -std=c11 -pedantic \
				 -Wno-unused-parameter -Wno-unused-variable -I$(LIB_DIR) -lpthread

# Флаги сборки debug/release
ifeq ($(BUILD), debug)
	CFLAGS := $(COMMON_CFLAGS) -g -O0 -fsanitize=address -fno-omit-frame-pointer
else ifeq ($(BUILD), release)
	CFLAGS := $(COMMON_CFLAGS) -O2
else
	$(error Unknown build type: $(BUILD))
endif

.PHONY: all clean debug release

# Основная сборка
all: $(TARGET)

$(TARGET): $(SRCS)
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ $(SRCS)

# Псевдонимы для debug/release
debug:
	$(MAKE) BUILD=debug all

release:
	$(MAKE) BUILD=release all

# Очистка
clean:
	rm -rf ./out
