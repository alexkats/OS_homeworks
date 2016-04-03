#define _XOPEN_SOURCE 500
#define _POSIX_SOURCE

#include <helpers.h>
#include <bufio.h>
#include <unistd.h>
#include <netdb.h>
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

    if (listen(fd, -1) == -1) {
        prerr();
        return -1;
    }

    return fd;
}

int main(int argc, char** argv) {
    if (argc != 2 || argv[1] == NULL) {
        custom_prerr("Usage: ./netsh <port>");
        return -1;
    }

    const char* pid_file = "/home/alex/OS_homeworks/netsh/pid";

    int pid_fd = open(pid_file, O_RDWR|O_CREAT);

    if (pid_fd < 0) {
        prerr();
        return -1;
    }

    pid_t pid = fork();

    switch (pid) {
        case 0:
            break;
        case -1:
            prerr();
            close(pid_fd);
            unlink(pid_file);
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
            close(pid_fd);
            unlink(pid_file);
            return -1;
        default:
            exit(0);
            break;
    }

    dprintf(pid_fd, "%d\n", getpid());
    close(pid_fd); 
    int sock_fd = get_socket_fd(argv[1]);

    if (sock_fd == -1) {
        unlink(pid_file);
        return -1;
    }

    sleep(5);
    unlink(pid_file);

    return 0;
}
