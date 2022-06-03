CC = gcc
CFLAGS = -Wall -Wformat=2 -Wshadow -Wconversion -std=c11 -D_XOPEN_SOURCE=700 -D_GNU_SOURCE

.PHONY: all clean monochrome

all: nina

clean:
	$(RM) nina

monochrome: CFLAGS += -D_NO_COLOR_TERM
monochrome: nina

nina: nina.c
	$(CC) $(CFLAGS) $^ -o $@
