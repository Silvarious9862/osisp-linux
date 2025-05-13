BUILD ?= debug

# Каталоги
SRC_DIR := ./src
INC_DIR := ./include
OUT_DIR := ./out/$(BUILD)

# Имена бинарных файлов
TARGET_PARENT   := $(OUT_DIR)/parent
TARGET_PRODUCER := $(OUT_DIR)/producer
TARGET_CONSUMER := $(OUT_DIR)/consumer

# Исходные файлы
SRCS_PARENT   := $(SRC_DIR)/main.c $(SRC_DIR)/ipc.c
SRCS_PRODUCER := $(SRC_DIR)/producer.c $(SRC_DIR)/ipc.c
SRCS_CONSUMER := $(SRC_DIR)/consumer.c $(SRC_DIR)/ipc.c

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

.PHONY: all clean parent producer consumer debug release

all: $(TARGET_PARENT) $(TARGET_PRODUCER) $(TARGET_CONSUMER)

$(TARGET_PARENT): $(SRCS_PARENT)
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ $(SRCS_PARENT)

$(TARGET_PRODUCER): $(SRCS_PRODUCER)
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ $(SRCS_PRODUCER)

$(TARGET_CONSUMER): $(SRCS_CONSUMER)
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ $(SRCS_CONSUMER)

parent: $(TARGET_PARENT)
producer: $(TARGET_PRODUCER)
consumer: $(TARGET_CONSUMER)

debug:
	$(MAKE) BUILD=debug all

release:
	$(MAKE) BUILD=release all

clean:
	rm -rf ./out
