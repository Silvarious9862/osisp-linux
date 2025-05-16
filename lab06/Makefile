# По умолчанию сборка debug (для release запускайте: make release)
BUILD ?= debug

# Каталоги
SRC_DIR := ./src
LIB_DIR := ./include
OUT_DIR := ./out/$(BUILD)

# Имена бинарных файлов
TARGET_GEN := $(OUT_DIR)/gen
TARGET_VIEW := $(OUT_DIR)/view
TARGET_SORT := $(OUT_DIR)/sort_index

# Исходные файлы
SRCS_GEN := $(SRC_DIR)/gen.c
SRCS_VIEW := $(SRC_DIR)/view.c
SRCS_SORT := $(SRC_DIR)/sort_index.c $(SRC_DIR)/prep.c $(SRC_DIR)/scan.c $(SRC_DIR)/sort.c $(SRC_DIR)/merge.c $(SRC_DIR)/write.c $(SRC_DIR)/finish.c

# Компилятор
CC := gcc

# Общие CFLAGS
COMMON_CFLAGS := -D_GNU_SOURCE -D_POSIX_C_SOURCE=200112L -W -Wall -Wextra -std=c11 -pedantic -Wno-unused-parameter -Wno-unused-variable -I$(LIB_DIR) -lpthread

# Флаги зависят от типа сборки (debug или release)
ifeq ($(BUILD), debug)
	CFLAGS := $(COMMON_CFLAGS) -g -O0 -fsanitize=address -fno-omit-frame-pointer -g
else ifeq ($(BUILD), release)
	CFLAGS := $(COMMON_CFLAGS) -O2
else
	$(error Unknown build type: $(BUILD))
endif

.PHONY: all clean gen view sort debug release

# Цель all собирает все три программы
all: $(TARGET_GEN) $(TARGET_VIEW) $(TARGET_SORT)

# Сборка программы генерации
$(TARGET_GEN): $(SRCS_GEN)
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ $(SRCS_GEN)

# Сборка программы просмотра
$(TARGET_VIEW): $(SRCS_VIEW)
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ $(SRCS_VIEW)

# Сборка программы сортировки
$(TARGET_SORT): $(SRCS_SORT)
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ $(SRCS_SORT)

# Цели для сборки отдельных программ
gen: $(TARGET_GEN)
view: $(TARGET_VIEW)
sort: $(TARGET_SORT)

# Псевдонимы для разных типов сборки
debug:
	$(MAKE) BUILD=debug all

release:
	$(MAKE) BUILD=release all

# Очистка каталога с бинарными файлами
clean:
	rm -rf ./out
	rm -f test/testfile
