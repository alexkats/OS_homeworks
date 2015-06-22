#include <helpers.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const size_t size = 4096;
const char delimeter = ' ';

void reverse(char *word, size_t length)
{
    for (int i = 0; i < length / 2; i++)
    {
        char c = word[i];
        word[i] = word[length - i - 1];
        word[length - i - 1] = c;
    }
}

int main()
{
    char buf[size];
    char word[size + 1];
    size_t length = 0;
    ssize_t rhave = 0;
    ssize_t whave = 0;

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
                reverse(word, length);
                word[length++] = ' ';
                whave = write_(STDOUT_FILENO, word, length);

                if (whave == -1)
                {
                    fprintf(stderr, "%s\n", strerror(errno));
                    return 1;
                }

                if (whave < length)
                {
                    fprintf(stderr, "Cannot output whole word\n");
                    return 1;
                }

                length = 0;
            }
            else
                word[length++] = buf[i];
    }
    
    if (length > 0)
    {
        reverse(word, length);
        whave = write_(STDOUT_FILENO, word, length);
        
        if (whave == -1)
        {
            fprintf(stderr, "%s\n", strerror(errno));
            return 1;
        }

        if (whave < length)
        {
            fprintf(stderr, "Cannot output whole word\n");
            return 1;
        }
    }

    return 0;
}
