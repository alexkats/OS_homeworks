#include "helpers.h"

ssize_t read_(int fd, void* buf, size_t count)
{
	int have = 0;
	int res = 0;

	while (1)
	{
		have = read(fd, (char*) buf + res, count);

		if (have == 0)
			return res;
		else if (have == -1)
			return -1;
		else if (have == count)
			return have + res;

		res += have;
		count -= have;
	}
}

ssize_t write_(int fd, const void* buf, size_t count)
{
	int have = 0;
	int res = 0;

	while (1)
	{
		have = write(fd, (char*) buf + res, count);

		if (have == -1)
			return -1;
		else if (have == count)
			return have + res;

		res += have;
		count -= have;
	}
}

ssize_t read_until(int fd, void* buf, size_t count, char delimeter)
{
    int have = 0;
    int res = 0;

    while (1)
    {
        have = read(fd, (char*) buf + res, count);

        for (int i = 0; i < have; i++)
            if (((char*) buf)[res + i] == delimeter)
                return have + res;

        if (have == 0)
            return res;
        else if (have == -1)
            return -1;
        else if (have == count)
            return have + res;

        res += have;
        count -= have;
    }
}
