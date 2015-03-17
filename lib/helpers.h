#ifndef _H_HELPERS_
#define _H_HELPERS_

#include <unistd.h>

ssize_t read_(int fd, void* buf, size_t count);
ssize_t write_(int fd, const void* buf, size_t count);
ssize_t read_until(int fd, void* buf, size_t count, char delimeter);

#endif //_H_HELPERS
