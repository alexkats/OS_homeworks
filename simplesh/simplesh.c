#include <bufio.h>
#include <helpers.h>

const size_t size = 4096;
const size_t count = 100;

execargs_t* programs[100];
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

            i -= 2;
        }
        else if (command[i] == '|')
        {
            if (!found_program)
                program[curr] = '\0';

            args[argc][curr] = '\0';
            curr = 0;
            i++;

            while (command[i++] == ' ') {}

            i -= 2;
            found_program = 0;
            programs[all++] = exec_new(program, args, argc);

            fprintf(stdout, "%s\n%d\n", program, argc);

            for (int j = 0; j < argc; j++)
                fprintf(stdout, "%s\n", args[j]);
            argc = 0;
        }
        else
        {
            if (!found_program)
                program[curr] = command[i];

            args[argc][curr++] = command[i];
        }
    }

    programs[all++] = exec_new(program, args, argc);
    fprintf(stdout, "%s\n%d\n", program, argc);

    for (int i = 0; i < argc; i++)
        fprintf(stdout, "%s\n", args[i]);
}

void action(int num)
{
}

int main()
{
    signal(SIGINT, action);
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

        command[rhave - 1] = '\n';
        command[rhave++] = '\0';
        init(command, rhave);
        fprintf(stdout, "%s\n", command);

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
