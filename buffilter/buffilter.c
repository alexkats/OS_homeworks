#include "buffilter.h"

const size_t size = 4096;

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: filter <executable file> <arguments>\n");
        return 1;
    }

    buf_t *buf = buf_new(size);
    buf_t *other = buf_new(size);
    char last_arg[size];
    
    for (int i = 0; i < argc - 1; i++)
        argv[i] = argv[i + 1];

    argv[argc - 1] = last_arg;
    argv[argc] = NULL;
    ssize_t rhave = 0;
    ssize_t whave = 0;
    size_t length = 0;

    while (1)
    {
        rhave = buf_getline(STDIN_FILENO, buf, last_arg);

        if (rhave == 0)
            break;

        if (rhave == -1)
        {
            fprintf(stderr, "%s\n", strerror(errno));
            buf_free(buf);
            buf_free(other);
            return 1;
        }

        last_arg[rhave] = 0;
        int code = spawn(argv[0], argv);

        if (code == 0)
        {
            length = rhave;
            last_arg[length++] = '\n';
            whave = buf_write(STDOUT_FILENO, other, last_arg, length);

            if (whave == -1)
            {
                fprintf(stderr, "%s\n", strerror(errno));
                buf_free(buf);
                buf_free(other);
                return 1;
            }

            if (whave < length)
            {
                fprintf(stderr, "Cannot output whole argument\n");
                buf_free(buf);
                buf_free(other);
                return 1;
            }
        }
    }

    buf_free(buf);
    buf_free(other);

    return 0;
}
