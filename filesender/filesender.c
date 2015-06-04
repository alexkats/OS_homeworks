#define _XOPEN_SOURCE

#include <bufio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

const size_t size = 4096;

typedef struct addrinfo addrinfo;
typedef struct sockaddr_storage sock_stor;

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Wrong usage!\nUsage: ./filesender <port> <file>\n");
        return -1;
    }

    addrinfo hints;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    char* port = argv[1];
    char* name = argv[2];
    addrinfo * res;
    int fd, acc_fd, file_fd;
    
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

    sock_stor receiver;
    socklen_t len = sizeof(receiver);

    while (1)
    {
        if ((acc_fd = accept(fd, (struct sockaddr*) &receiver, &len)) == -1)
        {
            fprintf(stderr, "%s\n", strerror(errno));
            close(fd);
            return -1;
        }

        pid_t process_id = fork();

        if (process_id == -1)
        {
            fprintf(stderr, "%s\n", strerror(errno));
            close(fd);
            close(acc_fd);
            return -1;
        }

        if (process_id != 0)
        {
            close(acc_fd);
            continue;
        }

        if ((file_fd = open(name, O_RDONLY)) == -1)
        {
            fprintf(stderr, "%s\n", strerror(errno));
            close(fd);
            close(acc_fd);
            return -1;
        }

        buf_t* buf = buf_new(size);

        if (buf == NULL)
        {
            fprintf(stderr, "Couldn't allocate memory for buffer\n");
            close(fd);
            close(acc_fd);
            close(file_fd);
            return -1;
        }

        while (1)
        {
            ssize_t rhave = buf_fill(file_fd, buf, buf -> capacity);

            if (rhave == -1)
            {
                fprintf(stderr, "Error in reading\n");
                buf_free(buf);
                close(fd);
                close(acc_fd);
                close(file_fd);
                return -1;
            }

            if (rhave == 0)
            {
                buf_free(buf);
                close(fd);
                close(acc_fd);
                close(file_fd);
                return 0;
            }

            ssize_t whave = buf_flush(acc_fd, buf, buf -> size);

            if (whave == -1)
            {
                fprintf(stderr, "Error in writing\n");
                buf_free(buf);
                close(fd);
                close(acc_fd);
                close(file_fd);
                return -1;
            }
        }
    }

    return 0;
}
