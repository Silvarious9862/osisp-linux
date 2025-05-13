BUILD ?= debug

# Каталоги
SRC_DIR := ./src
INC_DIR := ./include
OUT_DIR := ./out/$(BUILD)

# Имена бинарных файлов
TARGET_PARENT := $(OUT_DIR)/parent
TARGET_CHILD  := $(OUT_DIR)/child

# Исходные файлы
SRCS_PARENT := $(SRC_DIR)/parent.c
SRCS_CHILD  := $(SRC_DIR)/child.c

# Компилятор
CC := gcc

# Общие CFLAGS
COMMON_CFLAGS := -D_GNU_SOURCE -D_POSIX_C_SOURCE=200112L -W -Wall -Wextra -std=c11 -pedantic -Wno-unused-parameter -Wno-unused-variable -I$(INC_DIR)

ifeq ($(BUILD), debug)
	CFLAGS := $(COMMON_CFLAGS) -g -O0 -fsanitize=address -fno-omit-frame-pointer
else ifeq ($(BUILD), release)
	CFLAGS := $(COMMON_CFLAGS) -O2
else
	$(error Unknown build type: $(BUILD))
endif

.PHONY: all clean parent child debug release

all: $(TARGET_PARENT) $(TARGET_CHILD)

$(TARGET_PARENT): $(SRCS_PARENT)
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ $(SRCS_PARENT)

$(TARGET_CHILD): $(SRCS_CHILD)
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ $(SRCS_CHILD)

parent: $(TARGET_PARENT)

child: $(TARGET_CHILD)

debug:
	$(MAKE) BUILD=debug all

release:
	$(MAKE) BUILD=release all

clean:
	rm -rf ./out
