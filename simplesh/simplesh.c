#include <bufio.h>
#include <helpers.h>

const size_t size = 4096;
const size_t count = 100;

execargs_t* programs[count];
int all;

void init(char* command, int len)
{
    char program[size];
    int argc = 0;
    all = 0;
    char* args[count];

    for (int i = 0; i < count; i++)
        args[i] = (char*) malloc(size);

    int from = 0;
    int curr = 0;
    int found_program = 0;

    while (command[from++] == ' ') {}

    for (int i = from - 1; i < len; i++)
    {
        if (command[i] == ' ' || command[i] == '\n' || command[i] == '\0')
        {
            if (!found_program)
            {
                program[curr] = '\0';
                found_program = 1;
            }

            args[argc++][curr] = '\0';
            curr = 0;
            i++;

            while (command[i++] == ' ') {}

            i--;
        }
        else if (command[i] == '|')
        {
            if (!found_program)
                program[curr] = '\0';

            args[argc++][curr] = '\0';
            curr = 0;
            i++;

            while (command[i++] == ' ') {}

            i--;
            found_program = 0;
            argc = 0;
            struct execargs_t* tmp = exec_new(program, args, argc);
            programs[all++] = tmp;
        }
        else
        {
            if (!found_program)
                program[curr] = command[i];

            args[argc][curr++] = command[i];
        }
    }

    struct execargs_t* tmp = exec_new(program, args, argc);
    programs[all++] = tmp;
}

int main()
{
    buf_t *buf = buf_new(size);

    if (buf == NULL)
    {
        fprintf(stderr, "Error in allocating memory");
        return -1;
    }

    ssize_t rhave = 0;

    while (1)
    {
        char command[size * 2];

        if (write(STDOUT_FILENO, "$", 1) == -1)
        {
            fprintf(stderr, "Error in output");
            return -1;
        }

        rhave = buf_getline(STDIN_FILENO, buf, command);

        if (rhave == 0)
            break;

        if (rhave == -1)
        {
            fprintf(stderr, "Error in input");
            return -1;
        }

        init(command, rhave);

        if (runpiped(programs, all) < 0)
        {
            fprintf(stderr, "Error in pipe");
            return -1;
        }
    }

    for (int i = 0; i < all; i++)
        exec_free(programs[i]);

    buf_free(buf);
    return 0;
}
