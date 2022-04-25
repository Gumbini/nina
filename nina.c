#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define A_RST "\033[0m"

#define A_CYN "\033[1;36m"
#define A_GRN "\033[1;32m"
#define A_RED "\033[1;31m"
#define A_WHT "\033[1;97m"
#define A_YLW "\033[1;33m"

static int inFd  = -1;
static int outFd = -1;

static void die(const char *msg) {
	if (errno) {
		fprintf(stderr, A_RED "[FAIL]" A_WHT " (%s) %s" A_RST "\n", msg, strerror(errno));
	} else {
		fprintf(stderr, A_RED "[FAIL]" A_WHT " %s" A_RST "\n", msg);
	}

	fprintf(stderr, "Use the \"--help\" switch for usage information.\n");

	if (inFd != -1) {
		close(inFd);
	}
	if (outFd != -1) {
		close(outFd);
	}

	exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {

	if (argc <= 0) {
		errno = EINVAL;
		die("No filename (argv[0])");
	}
	if (argc == 1) {
		errno = EINVAL;
		die("Missing argument");
	}

	if (strcmp(argv[1], "--help") == 0) {
		fprintf (stderr,
			A_YLW "        _               n" A_RST "on            |  Universal\n"
			A_YLW "       (_)              i" A_RST "nconvenient   |  data overwriting\n"
			A_YLW "  _ __  _ _ __   ____   n" A_RST "uke           |  and destruction\n"
			A_YLW " | '_ \\| | '_ \\ / _  |  a" A_RST "pplication    |  software\n"
			A_YLW " | | | | | | | | (_| |  " A_RST "\n"
			A_YLW " |_| |_|_|_| |_|\\____|  " A_CYN "(c) 2022 Gumbini" A_RST "\n\n"
			"This program is free software; you can redistribute it and/or\n"
			"modify it under the terms of the GNU General Public License (Version 2.0)\n"
			"as published by the Free Software Foundation.\n\n"
			"Usage: %s <file path>\n", argv[0]);

		return EXIT_SUCCESS;
	}
	
	outFd = open(argv[1], O_WRONLY);
	if (outFd == -1) {
		die("open");
	}

	struct stat statbuf;
	if (fstat(outFd, &statbuf) == -1) {
		die("fstat");
	}
	char writeBuf[statbuf.st_blksize];
	char inputBuf[8];

	fprintf(stderr, "File: %s | ", argv[1]);
	if (statbuf.st_size == 0) {
		fprintf(stderr, A_RED "Size: 0 Bytes (Empty file!) " A_RST "\n");
		die("Cannot nuke empty file!");
	}
	fprintf(stderr, "Size: %li Byte(s)" A_RST "\n", statbuf.st_size);

	fprintf(stderr, A_WHT "Proceed to nuke the file to nirvana?" A_RST " [yes/*]\n");

	if (fgets(inputBuf, 8, stdin) == NULL) {
		die("fgets");
	}
	inputBuf[strcspn(inputBuf, "\r\n")] = '\0';

	if (strcmp(inputBuf, "yes") != 0) {
		abort();
	}

	fprintf(stderr, A_WHT "Choose method" A_RST ": [0] /dev/zero, [1] /dev/urandom\n");
	if (fgets(inputBuf, 8, stdin) == NULL) {
		die("fgets");
	}
	inputBuf[strcspn(inputBuf, "\r\n")] = '\0';
	if (strcmp(inputBuf, "0") == 0) {
		inFd = open("/dev/zero", O_RDONLY);
	} else if (strcmp(inputBuf, "1") == 0) {
		inFd = open("/dev/urandom", O_RDONLY);
	} else {
		abort();
	}

	if (inFd == -1) {
		die("open");
	}

	for (off_t i = 0; i < statbuf.st_size; i += statbuf.st_blksize) {
		if (read(inFd, writeBuf, statbuf.st_blksize) == -1) {
			die("read");
		}
		if (write(outFd, writeBuf, statbuf.st_blksize) == -1) {
			die("write");
		}
	}
	fprintf(stderr, "[" A_GRN "SUCCESS" A_RST "] write: Overwrite data\n");

	if (fsync(outFd) == -1) {
		die("fsync");
	}
	fprintf(stderr, "[" A_GRN "SUCCESS" A_RST "] fsync: Synchronize state with storage device\n");

	close(inFd);
	close(outFd);

	return EXIT_SUCCESS;
}
