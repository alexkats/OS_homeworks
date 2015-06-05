#define _XOPEN_SOURCE 500
#define _GNU_SOURCE

#include <bufio.h>
#include <unistd.h>
#include <poll.h>
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
typedef struct sockaddr_storage sock_stor;
typedef struct pollfd pollfd;

const size_t size = 4096;
const int timeout = 1000;

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

void action(int num)
{
    if (num == SIGCHLD)
        wait(NULL);
}

pollfd fds[127 * 2 + 2];
buf_t * buf1[127];
buf_t * buf2[127];

int check_in(int a)
{
    return fds[a].revents & POLLIN;
}

int check_out(int a)
{
    return fds[a].revents & POLLOUT;
}

int check_much(int a)
{
    return fds[a].revents & POLLRDHUP;
}
    
void swap(int a, int b)
{
    pollfd tmp = fds[a];
    fds[a] = fds[b];
    fds[b] = tmp;
}

void swap_buf(int a, int b)
{
    buf_t * tmp = buf1[a];
    buf1[a] = buf1[b];
    buf1[b] = tmp;
    tmp = buf2[a];
    buf2[a] = buf2[b];
    buf2[b] = tmp;
}

void do_in(int a)
{
    if (check_in(a))
    {
        int buf_num = (a - 2) / 2;
        buf_fill(fds[a].fd, buf1[buf_num], 1);
        fds[a + 1].events |= POLLOUT;

        if (buf1[buf_num] -> size == buf1[buf_num] -> capacity)
        {
            fds[a].events |= POLLIN;
            fds[a].events ^= POLLIN;
        }
    }

    if (check_in(a + 1))
    {
        int buf_num = (a - 3) / 2;
        buf_fill(fds[a].fd, buf2[buf_num], 1);
        fds[a - 1].events |= POLLOUT;

        if (buf1[buf_num] -> size == buf1[buf_num] -> capacity)
        {
            fds[a].events |= POLLIN;
            fds[a].events ^= POLLIN;
        }
    }
}

void do_out(int a)
{
    if (check_out(a))
    {
        int buf_num = (a - 2) / 2;
        buf_fill(fds[a].fd, buf1[buf_num], 1);
        fds[a + 1].events |= POLLIN;

        if (buf1[buf_num] -> size == buf1[buf_num] -> capacity)
        {
            fds[a].events |= POLLOUT;
            fds[a].events ^= POLLOUT;
        }
    }

    if (check_out(a + 1))
    {
        int buf_num = (a - 3) / 2;
        buf_fill(fds[a].fd, buf2[buf_num], 1);
        fds[a - 1].events |= POLLIN;

        if (buf1[buf_num] -> size == buf1[buf_num] -> capacity)
        {
            fds[a].events |= POLLOUT;
            fds[a].events ^= POLLOUT;
        }
    }
}

int main(int argc, char** argv)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = action;
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

    fds[0].fd = fd1;
    fds[1].fd = fd2;
    fds[0].events = POLLIN;
    fds[1].events = POLLIN;
    sock_stor receiver1;
    sock_stor receiver2;
    socklen_t len1 = sizeof(receiver1);
    socklen_t len2 = sizeof(receiver2);
    int curr = 0;
    int state = 0;

    while (1)
    {
        int num = poll(fds, curr, timeout);
        
        if (num < 0 && errno != EINTR)
            return -1;

        if (num == 0)
            continue;

        for (int i = 0; i < curr; i++)
        {
            int num = i * 2 + 2;

            if (check_much(num) || check_much(num + 1))
            {
                close(fds[num].fd);
                close(fds[num + 1].fd);
                buf_free(buf1[i]);
                buf_free(buf2[i]);

                swap(num, curr * 2);
                swap(num + 1, curr * 2 + 1);
                fds[0].events |= POLLIN;
                fds[1].events |= POLLIN;
                curr--;
                swap_buf(i, curr);

                break;
            }

            do_in(num);
            do_out(num);
        }

        if (check_in(state))
        {
            if (state == 0)
            {
                if ((acc_fd1 = accept(fd1, (struct sockaddr*) &receiver1, &len1)) == -1)
                    return -1;

                state = 1;
                continue;
            }

            if ((acc_fd2 = accept(fd2, (struct sockaddr*) &receiver2, &len2)) == -1)
                return -1;

            state = 0;
            int num = curr * 2 + 2;
            fds[num].fd = fd1;
            fds[num + 1].fd = fd2;
            fds[num].events = fds[num + 1].events = (POLLIN | POLLRDHUP);
            buf1[curr] = buf_new(size);
            buf2[curr] = buf_new(size);
            
            if (++curr >= 127)
            {
                fds[0].events |= POLLIN;
                fds[0].events ^= POLLIN;
                fds[1].events |= POLLIN;
                fds[1].events ^= POLLIN;
            }
        }
    }

    close(fd1);
    close(fd2);

    return 0;
}
