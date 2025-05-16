BUILD ?= debug

# Каталоги
SRC_DIR := ./src
INC_DIR := ./include
OUT_DIR := ./out/$(BUILD)

# Имя бинарного файла для новой потоковой версии
TARGET := $(OUT_DIR)/threads

# Исходные файлы для новой архитектуры
SRCS := $(SRC_DIR)/main.c $(SRC_DIR)/producer.c $(SRC_DIR)/consumer.c $(SRC_DIR)/queue.c

# Компилятор
CC := gcc

# Общие CFLAGS
COMMON_CFLAGS := -D_GNU_SOURCE -D_POSIX_C_SOURCE=200112L -W -Wall -Wextra -std=c11 -pedantic -I$(INC_DIR)

ifeq ($(BUILD), debug)
	CFLAGS := $(COMMON_CFLAGS) -g -O0 -fsanitize=address -fno-omit-frame-pointer
else ifeq ($(BUILD), release)
	CFLAGS := $(COMMON_CFLAGS) -O2
else
	$(error Unknown build type: $(BUILD))
endif

.PHONY: all clean debug release

all: $(TARGET)

$(TARGET): $(SRCS)
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -pthread -o $@ $(SRCS)

debug:
	$(MAKE) BUILD=debug all

release:
	$(MAKE) BUILD=release all

clean:
	rm -rf ./out
