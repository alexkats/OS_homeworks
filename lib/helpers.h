#ifndef _H_HELPERS_
#define _H_HELPERS_

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

ssize_t read_(int fd, void* buf, size_t count);
ssize_t write_(int fd, const void* buf, size_t count);
ssize_t read_until(int fd, void* buf, size_t count, char delimeter);
int spawn(const char* file, char* const argv[]);

#endif //_H_HELPERS
