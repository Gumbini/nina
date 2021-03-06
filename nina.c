/*
 * Copyright (C) 2022 Martin Weinzierl (Gumbini)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License (Verison 2.0)
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
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

#ifdef _NO_COLOR_TERM

#define A_RST ""
#define A_UDL ""

#define A_CYN ""
#define A_GRN ""
#define A_RED ""
#define A_WHT ""
#define A_YLW ""

#else

#define A_RST "\033[0m"
#define A_UDL "\033[1;4m"

#define A_CYN "\033[1;36m"
#define A_GRN "\033[1;32m"
#define A_RED "\033[1;31m"
#define A_WHT "\033[1;97m"
#define A_YLW "\033[1;33m"

#endif

static int inFd  = -1;
static int outFd = -1;

static const char *inPath  = NULL;
static const char *outPath = NULL;

static char method = '-';
static bool nuke   = false;

static void die(const char *msg) {
	if (errno) {
		fprintf(stderr, "[" A_RED "EXIT %d" A_WHT "] (%s) %s" A_RST "\n", errno, strerror(errno), msg);
	} else {
		fprintf(stderr, "[" A_RED "EXIT" A_WHT "] %s" A_RST "\n", msg);
	}

	fprintf(stderr, "Use the \"--help\" switch to view correct usage.\n");

	if (inFd != -1) {
		close(inFd);
	}
	if (outFd != -1) {
		close(outFd);
	}

	exit(EXIT_FAILURE);
}

void parseArgs(int argc, char **argv) {
	// Illegal call to exec()
	if (argc <= 0 || *argv == NULL) {
		errno = EINVAL;
		die("No filename (argv[0])");
	}
	// Just the program path
	if (argc == 1) {
		errno = EINVAL;
		die("Missing argument(s)");
	}

	for (int i = 1; i < argc && argv[i] != NULL; i++) {
		if (strncmp(argv[i], "--", 2) != 0) {
			if (outPath == NULL) {
				outPath = argv[i];
				continue;
			}
			errno = ENOTSUP;
			die("Multiple file paths specified");
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
						  "Usage: %s <file path> [--nuke] [--zero | --random]\n",
					argv[0]);

			exit(EXIT_SUCCESS);
		}
		if (strcmp(argv[i], "--nuke") == 0) {
			if (!nuke) {
				nuke = true;
				continue;
			}
			errno = EINVAL;
			die("--nuke is already present");
		}
		if (strcmp(argv[i], "--zero") == 0) {
			switch (method) {
				case '-':
					method = 'z';
					continue;
				case 'z':
					errno = EINVAL;
					die("--zero is already present");
				default:
					errno = ENOTSUP;
					die("--zero: an other method is already specified");
			}
		}
		if (strcmp(argv[i], "--random") == 0) {
			switch (method) {
				case '-':
					method = 'r';
					continue;
				case 'r':
					errno = EINVAL;
					die("--random is already present");
				default:
					errno = ENOTSUP;
					die("--random: an other method is already specified");
			}
		}
		errno = ENOTSUP;
		die("Invalid switch provided");
	}
}

int main(int argc, char **argv) {
	parseArgs(argc, argv);

	if (outPath == NULL) {
		errno = ENOTSUP;
		die("Missing path argument");
	}

	// O_DSYNC: implicit call to fdatasync() after each call to write()
	outFd = open(outPath, O_WRONLY | O_DSYNC | O_NOFOLLOW);

	if (outFd == -1) {
		if (errno == ELOOP) {
			fprintf(stderr, "[" A_YLW "HINT" A_RST "] Have you tried nuking a " A_UDL "symbolic link" A_RST "?\n"
							"       This feature has been disabled because symlinks cannot be nuked.\n");
			char linkPath[128];
			ssize_t nbytes;
			if ((nbytes = readlink(outPath, linkPath, 128)) != -1) {
				fprintf(stderr,
						"       You might want to take a look at \"" A_UDL "%.*s" A_RST
						"\". (<-- The shown path may be truncated!)\n",
						(int) nbytes, linkPath);
			}
		}
		die("open(2)");
	}

	struct stat statbuf;
	if (fstat(outFd, &statbuf) == -1) {
		die("fstat(2)");
	}

	// These flags require the "_XOPEN_SOURCE 700" POSIX.1-2008 macro
	// S_IFMT: only consider file type
	switch (statbuf.st_mode & S_IFMT) {
		case S_IFREG:
			break;
		case S_IFCHR:
			if (!isatty(outFd)) {
				break;
			}
		default:
			errno = ENOTSUP;
			die("Unsupported file type");
	}

	char writeBuf[statbuf.st_blksize];
	char inputBuf[8];

	fprintf(stderr, "File: " A_GRN "%s" A_RST " | ", outPath);
	if (statbuf.st_size == 0) {
		fprintf(stderr, A_RED "Size: 0 Bytes (Empty file!) " A_RST "\n");
		die("Cannot nuke empty file");
	}
	fprintf(stderr, "Size: " A_UDL "%li" A_RST " Byte(s)\n", statbuf.st_size);

	if (!nuke) {
		fprintf(stderr, A_WHT "Nuke the file to nirvana?" A_RST " [yes/*]\n");

		if (fgets(inputBuf, 8, stdin) == NULL) {
			die("fgets(3)");
		}
		inputBuf[strcspn(inputBuf, "\r\n")] = '\0';

		if (strcmp(inputBuf, "yes") != 0) {
			abort();
		}
	}

	if (method == '-') {
		fprintf(stderr, A_WHT "Choose method" A_RST ": [0] /dev/zero, [1] /dev/urandom\n");
		if (fgets(inputBuf, 8, stdin) == NULL) {
			die("fgets(3)");
		}
		inputBuf[strcspn(inputBuf, "\r\n")] = '\0';
		if (strcmp(inputBuf, "0") == 0) {
			method = 'z';
		} else if (strcmp(inputBuf, "1") == 0) {
			method = 'r';
		}
	}

	switch (method) {
		case 'z':
			inPath = "/dev/zero";
			break;
		case 'r':
			inPath = "/dev/urandom";
			break;
		default:
			abort();
	}

	inFd = open(inPath, O_RDONLY);

	if (inFd == -1) {
		die("open(2)");
	}

	fprintf(stderr, "[" A_YLW " BEGIN " A_RST "] write(2): Overwrite data using synchronous I/O\n");

	for (off_t i = 0; i < statbuf.st_size; i += statbuf.st_blksize) {
		if (read(inFd, writeBuf, (size_t) statbuf.st_blksize) == -1) {
			die("read(2)");
		}
		if (write(outFd, writeBuf, (size_t) statbuf.st_blksize) == -1) {
			die("write(2)");
		}
	}
	fprintf(stderr, "[" A_GRN "SUCCESS" A_RST "] write(2): Overwrite data\n");

	if (fsync(outFd) == -1) {
		die("fsync(2)");
	}
	fprintf(stderr, "[" A_GRN "SUCCESS" A_RST "] fsync(2): Synchronize state with storage device\n");

	close(inFd);
	close(outFd);

	return EXIT_SUCCESS;
}
