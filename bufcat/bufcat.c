#include <bufio.h>

const size_t size = 4096;

int main()
{
    buf_t *buf = buf_new(size);
    ssize_t rhave = 0;
    ssize_t whave = 0;

    while (1)
    {
        rhave = buf_fill(STDIN_FILENO, buf, buf_capacity(buf));
        whave = buf_flush(STDOUT_FILENO, buf, buf_size(buf));

        if (rhave == -1 || whave == -1)
        {
            fprintf(stderr, "%s\n", strerror(errno));
            return 1;
        }

        if (rhave < buf_capacity(buf))
            break;
    }

    buf_free(buf);
    return 0;
}
