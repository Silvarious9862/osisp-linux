CC = gcc
# Флаги компилятора:
# -D_GNU_SOURCE - включает расширения GNU
# -D_POSIX_C_SOURCE=200112L - определяет стандарт POSIX для большей переносимости
# -W, -Wall, -Wextra - включают дополнительные предупреждения компилятора
# -std=c11 - используется стандарт C11
# -pedantic - требует строгого соответствия стандарту
# -Wno-unused-parameter, -Wno-unused-variable - отключают предупреждения для неиспользуемых параметров и переменных
CFLAGS = -D_GNU_SOURCE -D_POSIX_C_SOURCE=200112L -W -Wall -Wextra -std=c11 -pedantic -Wno-unused-parameter -Wno-unused-variable

TARGET = main
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
    $(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
    rm -f $(TARGET)

.PHONY: all clean
