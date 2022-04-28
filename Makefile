CC = gcc
CFLAGS = -Wall -Wformat=2 -Wshadow -Wconversion -std=c11

.PHONY: all clean

all: nina

clean:
	rm nina

nina: nina.c
	$(CC) $(CFLAGS) nina.c -o nina
