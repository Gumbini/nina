CC = gcc
CFLAGS = -std=gnu11 -Wall

.PHONY: all

all:
	$(CC) $(CFLAGS) nina.c -o nina
