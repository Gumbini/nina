CC = gcc
CFLAGS = -Wall -Wformat=2 -Wshadow -Wconversion -std=c11 -D_XOPEN_SOURCE=700

.PHONY: all clean

all: nina

clean:
	rm nina

nina: nina.c
	$(CC) $(CFLAGS) nina.c -o nina
