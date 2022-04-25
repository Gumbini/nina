CC = gcc
CFLAGS = -Wall -Wformat=2 -Wshadow -Wconversion -std=c11

.PHONY: all clean

all:
	$(CC) $(CFLAGS) nina.c -o nina

clean:
	rm nina
