#define _XOPEN_SOURCE 500
#define _POSIX_SOURCE

#include <helpers.h>
#include <bufio.h>
#include <unistd.h>
#include <netdb.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <sys/epoll.h>
#include <vector>

using namespace std;

typedef struct addrinfo addrinfo;
typedef struct sockaddr_storage sock_stor;
typedef struct sockaddr_in sock_in;

vector <execargs_t*> programs;

void parse(char* command, int len) {
    vector <char*> args;
    char* arg = (char*) malloc(len);
    char* program = (char*) malloc(len);
    int i = -1;
    int curr = 0;
    char found_program = 0;

    while (command[++i] == ' ') {}

    for (; i < len; i++) {
        if (command[i] == '\0') {
            programs.clear();
            return;
        } else if (command[i] == ' ' || command[i] == '\n') {
            if (curr == 0) {
                break;
            }

            if (!found_program) {
                program[curr] = '\0';
                found_program = 1;
            }

            arg[curr] = '\0';
            args.push_back(arg);
            curr = 0;

            while (command[++i] == ' ') {}

            i--;
        } else if (command[i] == '|') {
            if (!found_program) {
                program[curr] = '\0';
            }

            arg[curr] = '\0';
            args.push_back(arg);
            curr = 0;

            while (command[++i] == ' ') {}

            i--;
            found_program = 0;
            programs.push_back(exec_new(program, args, (int) args.size()));
            args.clear();
        } else {
            if (!found_program) {
                program[curr] = command[i];
            }

            arg[curr++] = command[i];
        }
    }

    programs.push_back(exec_new(program, args, (int) args.size()));
}

void prerr() {
    fprintf(stderr, "%s\n", strerror(errno));
}

void custom_prerr(const char* s) {
    fprintf(stderr, "%s\n", s);
}

int get_socket_fd(const char* port) {
    addrinfo hints;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    addrinfo* res;
    int fd;

    if (getaddrinfo(NULL, port, &hints, &res) != 0) {
        custom_prerr("Error occured in getaddrinfo");
        return -1;
    }

    for (addrinfo* it = res; ; it = it -> ai_next) {
        if (it == NULL) {
            custom_prerr("Couldn't bind any address");
            return -1;
        }

        int one = 1;

        if ((fd = socket(it -> ai_family, it -> ai_socktype, it -> ai_protocol)) == -1) {
            continue;
        }

        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == -1) {
            prerr();
            return -1;
        }

        if (bind(fd, it -> ai_addr, it -> ai_addrlen) == -1) {
            close(fd);
            continue;
        }

        break;
    }

    freeaddrinfo(res);
    return fd;
}

int become_daemon(const char* pid_file, const char* log_file) {
    pid_t pid = fork();

    switch (pid) {
        case 0:
            break;
        case -1:
            prerr();
            return -1;
        default:
            exit(0);
            break;
    }

    if (setsid() < 0) {
        return -1;
    }

    pid = fork();

    switch (pid) {
        case 0:
            break;
        case -1:
            prerr();
            return -1;
        default:
            exit(0);
            break;
    }
    
    int pid_fd = open(pid_file, O_RDWR|O_CREAT|O_EXCL, 0644);
    
    if (access(log_file, F_OK) == 0) {
        unlink(log_file);
    }

    int log_fd = open(log_file, O_RDWR|O_CREAT|O_EXCL, 0644);

    for (int i = sysconf(_SC_OPEN_MAX); i >= 0; i--) {
        if (i != pid_fd && i != log_fd) {
            close(i);
        }
    }

    int stdio_fd = open("/dev/null", O_RDWR);
    dup(stdio_fd);
    dup(stdio_fd);

    if (pid_fd < 0 || log_fd < 0) {
        prerr();
        return -1;
    }

    dprintf(pid_fd, "%d\n", getpid());
    close(pid_fd); 
    dprintf(log_fd, "Just test");
    setpgrp();
    return log_fd;
}

int make_non_blocking(int sock_fd) {
    int flags = fcntl(sock_fd, F_GETFL, 0);

    if (flags == -1) {
        prerr();
        return -1;
    }

    flags |= O_NONBLOCK;
    if (fcntl(sock_fd, F_SETFL, flags) == -1) {
        prerr();
        return -1;
    }

    return 0;
}

int main(int argc, char** argv) {
    if (argc != 2 || argv[1] == NULL) {
        custom_prerr("Usage: ./netsh <port>");
        return -1;
    }

    const char* pid_file = "/home/alex/OS_homeworks/netsh/pid";
    const char* log_file = "/home/alex/OS_homeworks/netsh/log";
    int log_fd;
    
    if ((log_fd = become_daemon(pid_file, log_file)) == -1) {
        return -1;
    }

    int sock_fd = get_socket_fd(argv[1]);

    if (sock_fd == -1) {
        unlink(pid_file);
        close(log_fd);
        return -1;
    }

    if (make_non_blocking(sock_fd) == -1) {
        unlink(pid_file);
        close(log_fd);
        close(sock_fd);
        return -1;
    }

    if (listen(sock_fd, -1) == -1) {
        prerr();
        unlink(pid_file);
        close(log_fd);
        close(sock_fd);
        return -1;
    }

    int epoll_fd = epoll_create1(0);

    if (epoll_fd == -1) {
        prerr();
        unlink(pid_file);
        close(log_fd);
        close(sock_fd);
        return -1;
    }

    sleep(5);
    close(sock_fd);
    close(log_fd);
    unlink(pid_file);

    return 0;
}
