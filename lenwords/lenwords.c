#include <helpers.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const size_t size = 4096;
const char delimeter = ' ';

int main()
{
    char buf[size];
    size_t length = 0;
    ssize_t rhave = 0;

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
            if (buf[i] == delimeter && i > 0 && buf[i - 1] == delimeter)
                fprintf(stdout, "%d\n", 0);
            else if (buf [i] == delimeter)
            {
                fprintf(stdout, "%d\n", (int) length);
                length = 0;
            }
            else
                length++;
    }

    if (length > 0)
        fprintf(stdout, "%d\n", (int) length);

    return 0;
}
