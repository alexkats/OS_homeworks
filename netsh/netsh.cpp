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
#include <string.h>

using namespace std;

typedef struct addrinfo addrinfo;
typedef struct sockaddr sockaddr;
typedef struct sockaddr_storage sock_stor;
typedef struct sockaddr_in sock_in;
typedef struct epoll_event epoll_event;

const int MAXEVENTS = 64;
const int SIZE = 65536;
const int BUF_SIZE = 4096;

vector <execargs_t*> programs;
int log_fd;

void parse(char* command, int len) {
    vector <char*> args;
    char* arg = (char*) malloc(len);
    char* program = (char*) malloc(len);
    int i = -1;
    int curr = 0;
    char found_program = 0;
    bool is_quote = 0;
    args.clear();
    programs.clear();

    while (command[++i] == ' ') {}

    for (; i < len; i++) {
        if (command[i] == '\0') {
            args.clear();
            programs.clear();
            return;
        } else if ((command[i] == ' ' && !is_quote) || command[i] == '\n') {
            if (curr == 0) {
                break;
            }

            if (!found_program) {
                program[curr] = '\0';
                found_program = 1;
            }

            arg[curr] = '\0';
            char* tmp = (char*) malloc(len);
            strcpy(tmp, arg);
            args.push_back(tmp);
            curr = 0;

            if (command[i] == '\n') {
                break;
            }

            while (command[++i] == ' ') {}

            i--;
        } else if (command[i] == '|') {
            if (!found_program) {
                program[curr] = '\0';
            }

            if (curr != 0) {
                arg[curr] = '\0';
                char* tmp = (char*) malloc(len);
                strcpy(tmp, arg);
                args.push_back(tmp);
                curr = 0;
            }

            while (command[++i] == ' ') {}

            i--;
            found_program = 0;
            programs.push_back(exec_new(program, args, (int) args.size()));
            args.clear();
        } else {
            if (command[i] == '\'' || command[i] == '\"') {
                is_quote = !is_quote;
                continue;
            }

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

void dprerr() {
    dprintf(log_fd, "%s\n", strerror(errno));
}

void custom_dprerr(const char* s) {
    dprintf(log_fd, "%s\n", s);
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
        custom_dprerr("Error occured in getaddrinfo");
        return -1;
    }

    for (addrinfo* it = res; ; it = it -> ai_next) {
        if (it == NULL) {
            custom_dprerr("Couldn't bind any address");
            return -1;
        }

        int one = 1;

        if ((fd = socket(it -> ai_family, it -> ai_socktype, it -> ai_protocol)) == -1) {
            continue;
        }

        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == -1) {
            dprerr();
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

    if (pid_fd == -1) {
        unlink(pid_file);
        return -1;
    }
    
    if (access(log_file, F_OK) == 0) {
        unlink(log_file);
    }

    int log_fd = open(log_file, O_RDWR|O_CREAT|O_EXCL, 0644);

    if (log_fd == -1) {
        close(pid_fd);
        unlink(pid_file);
        prerr();
        return -1;
    }

    for (int i = sysconf(_SC_OPEN_MAX); i >= 0; i--) {
        if (i != pid_fd && i != log_fd) {
            close(i);
        }
    }

    int stdio_fd = open("/dev/null", O_RDWR);
    dup(stdio_fd);
    dup(stdio_fd);

    dprintf(pid_fd, "%d\n", getpid());
    close(pid_fd); 

    if (setpgrp() == -1) {
        unlink(pid_file);
        dprerr();
        close(log_fd);
        return -1;
    }

    return log_fd;
}

int make_non_blocking(int sock_fd) {
    int flags = fcntl(sock_fd, F_GETFL, 0);

    if (flags == -1) {
        dprerr();
        return -1;
    }

    flags |= O_NONBLOCK;
    if (fcntl(sock_fd, F_SETFL, flags) == -1) {
        dprerr();
        return -1;
    }

    return 0;
}

int main(int argc, char** argv) {
    if (argc != 2 || argv[1] == NULL) {
        custom_prerr("Usage: ./netsh <port>");
        return -1;
    }

    buf_t* buf = buf_new(BUF_SIZE);

    if (buf == NULL) {
        custom_prerr("Error in allocating memory");
        return -1;
    }

    const char* pid_file = "/tmp/netsh.pid";
    const char* log_file = "/tmp/netsh.log";
    //const char* pid_file = "/home/alex/OS_homeworks/netsh/pid";
    //const char* log_file = "/home/alex/OS_homeworks/netsh/log";
    
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
        dprerr();
        unlink(pid_file);
        close(log_fd);
        close(sock_fd);
        return -1;
    }

    int epoll_fd = epoll_create1(0);

    if (epoll_fd == -1) {
        dprerr();
        unlink(pid_file);
        close(log_fd);
        close(sock_fd);
        return -1;
    }

    epoll_event event;
    epoll_event* events;

    event.data.fd = sock_fd;
    event.events = EPOLLIN|EPOLLET;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, &event) == -1) {
        dprerr();
        unlink(pid_file);
        close(log_fd);
        close(sock_fd);
        close(epoll_fd);
        return -1;
    }

    events = (epoll_event*) calloc(MAXEVENTS, sizeof event);

    while (1) {
        int n = epoll_wait(epoll_fd, events, MAXEVENTS, -1);

        for (int i = 0; i < n; i++) {
            if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN))) {
                custom_dprerr("Epoll error");
                close(events[i].data.fd);
                continue;
            }

            if (sock_fd == events[i].data.fd) {
                while (1) {
                    sockaddr in_addr;
                    socklen_t in_len = sizeof in_addr;
                    int in_fd = accept(sock_fd, &in_addr, &in_len);

                    if (in_fd == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break;
                        }

                        dprerr();
                        break;
                    }

                    char hbuf[NI_MAXHOST];
                    char sbuf[NI_MAXSERV];

                    if (getnameinfo(&in_addr, in_len, hbuf, sizeof hbuf, sbuf, sizeof sbuf, NI_NUMERICHOST|NI_NUMERICSERV) == 0) {
                        dprintf(log_fd, "Accepted connection on descriptor %d (host = %s, port = %s)\n", in_fd, hbuf, sbuf);
                    }

                    if (make_non_blocking(in_fd) == -1) {
                        unlink(pid_file);
                        close(log_fd);
                        close(sock_fd);
                        close(epoll_fd);
                        return -1;
                    }

                    event.data.fd = in_fd;
                    event.events = EPOLLIN|EPOLLET;

                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, in_fd, &event) == -1) {
                        dprerr();
                        unlink(pid_file);
                        close(log_fd);
                        close(sock_fd);
                        close(epoll_fd);
                        return -1;
                    }
                }

                continue;
            }

            ssize_t rhave = 0;
            char command[SIZE];

            rhave = buf_getline(events[i].data.fd, buf, command);

            if (rhave == -1) {
                continue;
            }

            parse(command, rhave);

            if (runpiped(programs, programs.size(), events[i].data.fd, events[i].data.fd, log_fd) < 0) {
                custom_dprerr("Error in pipe");
                unlink(pid_file);
                close(log_fd);
                close(sock_fd);
                close(epoll_fd);
                return -1;
            }

            close(events[i].data.fd);
        }
    }

    close(epoll_fd);
    free(events);
    buf_free(buf);
    close(sock_fd);
    close(log_fd);
    unlink(pid_file);

    return 0;
}
