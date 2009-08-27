#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "posix.h"


int main(int argc, char** argv) {
	int fd_input;
	int fd_output;
	
	ssize_t size;
	size_t size_half;
	
	if (argc != 3) {
		
		return 0;
	}
	
	/* Opening files */
	fd_input = open(argv[1], O_RDONLY);
	
	if (fd_input == -1) {
		perror("main() open input file:");
		return 1;
	}
	
	fd_output = open(argv[2], O_RDWR|O_CREAT, 0644);
	
	if (fd_output == -1) {
		perror("main() open output file:");
		return 1;
	}
	
	/* Getting size of input file */
	size = fsize(fd_input);
	
	if (size == -1) return 1;

	size_half = size / 2;
	
	/* Copy stuff */
	if (fcopy(fd_output, fd_input, 0, 0, size) == -1) return -1;
	
	if (fcopy(STDOUT_FILENO, fd_input, FCOPY_NO_SEEK, size_half, size_half) == -1) return 1;
	if (fcopy(STDOUT_FILENO, fd_input, FCOPY_NO_SEEK, 0, size_half) == -1) return 1;
	
	if (fcopy(STDOUT_FILENO, fd_input, FCOPY_NO_SEEK, 0, size_half) == -1) return 1;
	
	if (fcopy(fd_output, fd_input, 11, size-10, 10) == -1) return 1;
	
	if (ftruncate(fd_output, 21) == -1) {
		perror("main() ftruncate:");
		return 1;
	}
	
	if (fcopy(STDOUT_FILENO, fd_output, FCOPY_NO_SEEK, 0, FCOPY_READ_ALL) == -1) return 1;
	
	/* Closing files */
	if (close(fd_input) == -1) {
		perror("main() closing input file:");
		return 1;
	}
	
	if (close(fd_output) == -1) {
		perror("main() closing output file:");
		return 1;
	}
	
	return 0;
}

ssize_t fsize(int fd) {
	ssize_t size;

	/* Seeking end */
	size = lseek(fd, 0, SEEK_END);
	
	if (size == -1) {
		perror("fsize() lseek:");
		return -1;
	}
	
	/* Going back to beginning */
	if (lseek(fd, 0, SEEK_SET) == -1) {
		perror("fisze() lseek:");
		return -1;
	}
	
	return size;
}

ssize_t fcopy(int fd_output, int fd_input, off_t offset_output, off_t offset_input, size_t size_max) {
	char buffer[BUFFER_SIZE];
	ssize_t size_read;
	ssize_t size_write;
	size_t size_sum;
	
	if (offset_input != FCOPY_NO_SEEK)
		if (lseek(fd_input, offset_input, SEEK_SET) == -1) {
			perror("fcopy() lseek input:");
			return -1;
		}
	
	if (offset_output != FCOPY_NO_SEEK)
		if (lseek(fd_output, offset_output, SEEK_SET) == -1) {
			perror("fcopy() lseek output:");
			return -1;
		}
	
	size_sum = 0;
	
	do {
		size_read = read(fd_input, buffer, BUFFER_SIZE);
		
		if (size_read == -1) {
			perror("fcopy() read:");
			return -1;
		}
		
		if (size_max != FCOPY_READ_ALL && size_sum + size_read > size_max)
			size_read -= (size_sum + size_read) - size_max;
		
		size_sum += size_read;
	
		size_write = write(fd_output, buffer, size_read);
		
		if (size_write == -1) {
			perror("fcopy() write:");
			return -2;
		}
		
	} while (size_read != 0);
	
	return size_sum;
}
