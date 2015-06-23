#define _XOPEN_SOURCE 500

#include <bufio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <netdb.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>

typedef struct addrinfo addrinfo;
typedef struct sockaddr_in sock_in;

const size_t size = 4096;

int get_socket_fd(const char* port)
{
    addrinfo hints;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    addrinfo * res;
    int fd;
    
    if (getaddrinfo(NULL, port, &hints, &res) != 0)
    {
        fprintf(stderr, "Error occured in getaddrinfo\n");
        return -1;
    }

    for (addrinfo* it = res; ; it = it -> ai_next)
    {
        if (it == NULL)
        {
            fprintf(stderr, "Couldn't bind any address\n");
            return -1;
        }

        int one = 1;

        if ((fd = socket(it -> ai_family, it -> ai_socktype, it -> ai_protocol)) == -1)
            continue;

        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == -1)
        {
            fprintf(stderr, "%s\n", strerror(errno));
            return -1;
        }

        if (bind(fd, it -> ai_addr, it -> ai_addrlen) == -1)
        {
            close(fd);
            continue;
        }

        break;
    }

    freeaddrinfo(res);

    if (listen(fd, -1) == -1)
    {
        fprintf(stderr, "%s\n", strerror(errno));
        return -1;
    }

    return fd;
}

int main(int argc, char** argv)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sigaction(SIGCHLD, &sa, NULL);

    if (argc != 3)
    {
        fprintf(stderr, "Wrong uasge\nUsage: ./forking <port 1> <port 2>\n");
        return -1;
    }

    int fd1, fd2, acc_fd1, acc_fd2;

    if ((fd1 = get_socket_fd(argv[1])) == -1)
        return -1;

    if ((fd2 = get_socket_fd(argv[2])) == -1)
        return -1;

    sock_in receiver1;
    sock_in receiver2;
    socklen_t len1 = sizeof(receiver1);
    socklen_t len2 = sizeof(receiver2);
    buf_t* buf = buf_new(size);

    if (buf == NULL)
    {
        fprintf(stderr, "Couldn'y allocate memory for buffer\n");
        close(fd1);
        close(fd2);
        return -1;
    }

    while (1)
    {
        if ((acc_fd1 = accept(fd1, (struct sockaddr*) &receiver1, &len1)) == -1)
        {
            fprintf(stderr, "%s\n", strerror(errno));
            close(fd1);
            close(fd2);
            return -1;
        }

        if ((acc_fd2 = accept(fd2, (struct sockaddr*) &receiver2, &len2)) == -1)
        {
            fprintf(stderr, "%s\n", strerror(errno));
            close(fd1);
            close(fd2);
            return -1;
        }

        pid_t process_id = fork();

        if (process_id == -1)
        {
            fprintf(stderr, "%s\n", strerror(errno));
            close(fd1);
            close(fd2);
            close(acc_fd1);
            close(acc_fd2);
            return -1;
        }

        if (process_id == 0)
        {
            while (1)
            {
                ssize_t rhave = buf_fill(acc_fd1, buf, 1);

                if (rhave == -1)
                {
                    fprintf(stderr, "Error in reading\n");
                    buf_free(buf);
                    close(fd1);
                    close(fd2);
                    close(acc_fd1);
                    close(acc_fd2);
                    _exit(-1);
                }

                if (rhave == 0)
                   _exit(0);
            
                ssize_t whave = buf_flush(acc_fd2, buf, buf -> size);

                if (whave == -1)
                {
                    fprintf(stderr, "Error in writing\n");
                    buf_free(buf);
                    close(fd1);
                    close(fd2);
                    close(acc_fd1);
                    close(acc_fd2);
                    _exit(-1);
                }
            }
        }

        process_id = fork();

        if (process_id == -1)
        {
            fprintf(stderr, "%s\n", strerror(errno));
            close(fd1);
            close(fd2);
            close(acc_fd1);
            close(acc_fd2);
            return -1;
        }

        if (process_id != 0)
        {
            close(acc_fd1);
            close(acc_fd2);
            continue;
        }

        while (1)
        {
            ssize_t rhave = buf_fill(acc_fd2, buf, 1);

            if (rhave == -1)
            {
                fprintf(stderr, "Error in reading\n");
                buf_free(buf);
                close(fd1);
                close(fd2);
                close(acc_fd1);
                close(acc_fd2);
                _exit(-1);
            }

            if (rhave == 0)
                _exit(0);

            ssize_t whave = buf_flush(acc_fd1, buf, buf -> size);

            if (whave == -1)
            {
                fprintf(stderr, "Error in writing\n");
                buf_free(buf);
                close(fd1);
                close(fd2);
                close(acc_fd1);
                close(acc_fd2);
                _exit(-1);
            }
        }
    }

    buf_free(buf);

    return 0;
}
