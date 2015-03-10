#include <unistd.h>

ssize_t read_(int fd, void* buf, size_t count)
{
	int have = 0;
	int res = 0;

	while (1)
	{
		have = read(fd, (char*) buf + res, count);

		switch (have)
		{
			case 0:
				return res;
			case -1:
				return -1;
			case count:
				return have + res;
		}

		res += have;
		count -= have;
	}
}

ssize_t write_(int fd, void* buf, size_t count)
{
	int have = 0;
	int res = 0;

	while (1)
	{
		have = write(fd, (char*) buf + res, count);

		switch (have)
		{
			case -1:
				return -1;
			case count:
				return have + res;
		}

		res += have;
		count -= have;
	}
}
