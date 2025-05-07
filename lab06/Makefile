# Компилятор и флаги
CC = gcc
CFLAGS = -W -Wall -Wextra -std=c11 -pedantic -Wno-unused-parameter -Wno-unused-variable
LDFLAGS = -pthread

# Исполняемые файлы
TARGETS = gen sort_index view

# Основное правило
all: $(TARGETS)

# Правила сборки
gen: gen.c
	$(CC) $(CFLAGS) -o $@ $^

sort_index: sort_index.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

view: view.c
	$(CC) $(CFLAGS) -o $@ $^

# Правило очистки
clean:
	rm -f $(TARGETS) *.o

# Правило тестирования генерации файла
test_gen: gen
	./gen test_index.dat 256

# PHONY-цели
.PHONY: all clean test_gen
