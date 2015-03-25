#include "filter.h"

const size_t size = 4096;
const char delimeter = '\n';

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: filter <executable file> <arguments>\n");
        return 1;
    }

    char buf[size];
    char last_arg[size];
    char** arguments = (char**) malloc(sizeof(char*) * (argc + 1));
    memcpy(arguments, argv + 1, sizeof(char**) * (argc - 1));
    arguments[argc - 1] = last_arg;
    arguments[argc] = NULL;
    ssize_t rhave = 0;
    ssize_t whave = 0;
    size_t length = 0;

    while (1)
    {
        rhave = read_until(STDIN_FILENO, buf, size, delimeter);

        if (rhave == 0)
            break;

        if (rhave == -1)
        {
            fprintf(stderr, "%s\n", strerror(errno));
            return 1;
        }

        for (int i = 0; i < rhave; i++)
            if (buf[i] == delimeter)
            {
                if (length == 0)
                    continue;

                last_arg[length] = 0;
                int code = spawn(argv[1], arguments);

                if (code == 0)
                {
                    last_arg[length++] = '\n';
                    whave = write_(STDOUT_FILENO, last_arg, length);

                    if (whave == -1)
                    {
                        fprintf(stderr, "%s\n", strerror(errno));
                        return 1;
                    }

                    if (whave < length)
                    {
                        fprintf(stderr, "Cannot output whole argument\n");
                        return 1;
                    }
                }

                length = 0;
            }
            else
                last_arg[length++] = buf[i];
    }

    if (length > 0)
    {
        last_arg[length] = 0;
        int code = spawn(argv[1], arguments);

        if (code == 0)
        {
            last_arg[length++] = '\n';
            whave = write_(STDOUT_FILENO, last_arg, length);

            if (whave == -1)
            {
                fprintf(stderr, "%s\n", strerror(errno));
                return 1;
            }

            if (whave < length)
            {
                fprintf(stderr, "cannot output whole argument\n");
                return 1;
            }
        }
    }

    return 0;
}
