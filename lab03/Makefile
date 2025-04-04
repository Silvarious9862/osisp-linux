# Имя исполняемых файлов
PARENT_EXEC = parent
CHILD_EXEC = child

# Компилятор
CC = gcc

# Ключи компиляции
CFLAGS = -W -Wall -Wextra -std=c11 -pedantic -Wno-unused-parameter -Wno-unused-variable

# Файлы исходного кода
PARENT_SRC = parent.c
CHILD_SRC = child.c

# Задачи Make
all: $(PARENT_EXEC) $(CHILD_EXEC)

$(PARENT_EXEC): $(PARENT_SRC)
	$(CC) $(CFLAGS) -o $(PARENT_EXEC) $(PARENT_SRC)

$(CHILD_EXEC): $(CHILD_SRC)
	$(CC) $(CFLAGS) -o $(CHILD_EXEC) $(CHILD_SRC)

clean:
	rm -f $(PARENT_EXEC) $(CHILD_EXEC)
