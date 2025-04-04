# Название исполняемых файлов
PRODUCER = producer
CONSUMER = consumer
MAIN = main

# Компилятор и ключи
CC = gcc
CFLAGS = -W -Wall -Wextra -std=c11 -pedantic -Wno-unused-parameter -Wno-unused-variable

# Цели сборки
all: $(MAIN) $(PRODUCER) $(CONSUMER)

$(MAIN): $(MAIN).c
	$(CC) $(CFLAGS) -o $(MAIN) $(MAIN).c

$(PRODUCER): $(PRODUCER).c
	$(CC) $(CFLAGS) -o $(PRODUCER) $(PRODUCER).c

$(CONSUMER): $(CONSUMER).c
	$(CC) $(CFLAGS) -o $(CONSUMER) $(CONSUMER).c

# Очистка
clean:
	rm -f $(MAIN) $(PRODUCER) $(CONSUMER)
