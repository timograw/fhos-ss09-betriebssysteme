/* Posix.h */
#ifndef POSIX_H
#define POSIX_H

#define BUFFER_SIZE 16

#define FCOPY_READ_ALL -1
#define FCOPY_NO_SEEK -1

ssize_t fsize(int file_descriptor);
ssize_t fcopy(int fd_output, int fd_input, off_t offset_output, off_t offset_input, size_t size_max);

#endif

