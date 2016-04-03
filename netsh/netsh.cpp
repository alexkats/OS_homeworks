#define _XOPEN_SOURCE 500
#define _POSIX_SOURCE

#include <helpers.h>
#include <bufio.h>
#include <unistd.h>
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

int main(int argc, char** argv) {
    if (argc != 2 || argv[1] == NULL) {
        custom_prerr("Usage: ./netsh <port>");
        return -1;
    }

    

    // int pid_fd = open("/tmp/netsh.pid", O_RDWR|O_CREAT);
    int pid_fd = open("/home/alex/OS_homeworks/netsh/pid", O_RDWR|O_CREAT);

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

    dprintf(pid_fd, "%d\n", getpid());
    close(pid_fd);



    sleep(5);
    // unlink("/tmp/netsh.pid");
    unlink("/home/alex/OS_homeworks/netsh/pid");

    return 0;
}
