CC = gcc
CFLAGS = -D_GNU_SOURCE -D_POSIX_C_SOURCE=200112L -W -Wall -Wextra -std=c11 -pedantic -Wno-unused-parameter -Wno-unused-variable -Wno-deprecated-declarations -g
LDFLAGS = -lmagic -lcrypto

TARGET = bdupes
SRCS = main.c args.c result.c selection.c deletion.c filter_size.c filter_mime.c filter_hash.c filter_cmp.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
