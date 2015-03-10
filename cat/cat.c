#include <helpers.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const size_t size = 4096;

int main ()
{
	char buf[size];
	ssize_t rhave = 0;
	ssize_t whave = 0;

	while (1)
	{
		rhave = read_(STDIN_FILENO, buf, size);

		if (rhave == -1)
		{
			fprintf(stderr, "%s\n", strerror(errno));
			return 1;
		}

		whave = write_(STDOUT_FILENO, buf, rhave);

		if (whave == -1)
		{
			fprintf(stderr, "%s\n", strerror(errno));
			return 1;
		}

		if (rhave > whave)
		{
			fprintf(stderr, "Cannot output everything in file\n");
			return 1;
		}

		if (rhave < size)
			return 0;
	}
}
