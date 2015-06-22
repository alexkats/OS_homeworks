#ifndef _H_HELPERS_
#define _H_HELPERS_
#define _POSIX_SOURCE
#define _GNU_SOURCE

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

ssize_t read_(int fd, void* buf, size_t count);
ssize_t write_(int fd, const void* buf, size_t count);
ssize_t read_until(int fd, void* buf, size_t count, char delimeter);
int spawn(const char* file, char* const argv[]);

struct execargs_t;
typedef struct execargs_t execargs_t;

struct execargs_t *exec_new(char*, char**, int);
void exec_free(execargs_t*);
int exec(execargs_t *);
int runpiped(execargs_t **, size_t);

#endif //_H_HELPERS
