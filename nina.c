/*
 * Copyright (C) 2022 Martin Weinzierl (Gumbini)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License (Verison 2.0)
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
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

static const char *path = NULL;

static char method = '-';
static bool nuke   = false;

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

void parseArgs(int argc, char **argv) {
	if (argc <= 0 || *argv == NULL) { // Illegal call to exec()
		errno = EINVAL;
		die("No filename (argv[0])");
	}
	if (argc == 1) { // Just the program path
		errno = EINVAL;
		die("Missing argument");
	}

	for (int i = 1; i < argc && argv[i] != NULL; i++) {
		if (strncmp(argv[i], "--", 2) != 0) {
			if (path == NULL) {
				path = argv[i];
				continue;
			}
			die("Only one path at a time...");
		}

		if (strcmp(argv[i], "--help") == 0) {
			fprintf(stderr,
					A_YLW "        _               n" A_RST "on            |  Universal\n" A_YLW
						  "       (_)              i" A_RST "nconvenient   |  data overwriting\n" A_YLW
						  "  _ __  _ _ __   ____   n" A_RST "uke           |  and destruction\n" A_YLW
						  " | '_ \\| | '_ \\ / _  |  a" A_RST "pplication    |  software\n" A_YLW
						  " | | | | | | | | (_| |  " A_RST "\n" A_YLW " |_| |_|_|_| |_|\\____|  " A_CYN
						  "(c) 2022 Gumbini" A_RST "\n\n"
						  "This program is free software; you can redistribute it and/or\n"
						  "modify it under the terms of the GNU General Public License (Version 2.0)\n"
						  "as published by the Free Software Foundation.\n\n"
						  "Usage: %s <file path>\n",
					argv[0]);

			exit(EXIT_SUCCESS);
		}
		if (strcmp(argv[i], "--nuke") == 0) {
			nuke = true;
			continue;
		}
		if (strcmp(argv[i], "--zero") == 0) {
			if (method == '-') {
				method = 'z';
				continue;
			}
			die("--zero: Method already specified");
		}
		if (strcmp(argv[i], "--random") == 0) {
			if (method == '-') {
				method = 'r';
				continue;
			}
			die("--random: Method already specified");
		}
	}
}

int main(int argc, char **argv) {
	parseArgs(argc, argv);

	outFd = open(path, O_WRONLY);
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

	if (!nuke) {
		if (fgets(inputBuf, 8, stdin) == NULL) {
			die("fgets");
		}
		inputBuf[strcspn(inputBuf, "\r\n")] = '\0';

		if (strcmp(inputBuf, "yes") != 0) {
			abort();
		}
	}

	if (method == '-') {
		fprintf(stderr, A_WHT "Choose method" A_RST ": [0] /dev/zero, [1] /dev/urandom\n");
		if (fgets(inputBuf, 8, stdin) == NULL) {
			die("fgets");
		}
		inputBuf[strcspn(inputBuf, "\r\n")] = '\0';
		if (strcmp(inputBuf, "0") == 0) {
			method = 'z';
		} else if (strcmp(inputBuf, "1") == 0) {
			method = 'r';
		}
	}

	const char *inputPath = NULL;
	switch (method) {
		case 'z':
			inputPath = "/dev/zero";
			break;
		case 'r':
			inputPath = "/dev/urandom";
			break;
		default:
			abort();
	}

	inFd = open(inputPath, O_RDONLY);

	if (inFd == -1) {
		die("open");
	}

	for (off_t i = 0; i < statbuf.st_size; i += statbuf.st_blksize) {
		if (read(inFd, writeBuf, (size_t) statbuf.st_blksize) == -1) {
			die("read");
		}
		if (write(outFd, writeBuf, (size_t) statbuf.st_blksize) == -1) {
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
