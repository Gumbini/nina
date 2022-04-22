#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

#define A_WHT "\033[1;97m"
#define A_RST "\033[0m"

static void die(const char *msg) {
	if (errno) {
		perror(msg);
	} else {
		fprintf(stderr, "[no errno] %s\n", msg);
	}

	exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
	if (argc <= 0) {
		errno = EINVAL;
		die("No filename (argv[0])");
	}

	fprintf(stderr, A_WHT "nina" A_RST " - If she was good at one thing, it was destroying...\n");

	if (argc == 1) {
		errno = EINVAL;
		die("Missing argument");
	}

	int fd = open(argv[1], O_WRONLY | O_CLOEXEC);
	if (fd == -1) {
		die("open");
	}
	fprintf(stderr, "File: \"%s\"\n", argv[1]);

	struct stat statbuf;
	if (fstat(fd, &statbuf) == -1) {
		die("fstat");
	}
	fprintf(stderr, "Size: %li Bytes. Proceed to nuke the file to nirvana? [yes/*]\n",
			statbuf.st_size);

	char input[8];
	if (fgets(input, 8, stdin) == NULL) {
		die("fgets");
	}
	input[strcspn(input, "\r\n")] = '\0';

	if (strcmp(input, "yes") != 0) {
		abort();
	}

	char buf[statbuf.st_blksize];
	memset(buf, '\0', statbuf.st_blksize);

	for (off_t i = 0; i < statbuf.st_size; i += statbuf.st_blksize) {
		if (write(fd, buf, statbuf.st_blksize) == -1) {
			die("write");
		}
	}
	fprintf(stderr, "Successfully overwritten.\n");

	if (fsync(fd) == -1) {
		die("fsync");
	}
	fprintf(stderr, "Kernel transfered all modified data to the disk device.\n");

	close(fd);
	return EXIT_SUCCESS;
}
