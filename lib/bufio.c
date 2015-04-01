#include "bufio.h"

#ifdef DEBUG
    #define CHECK(condition) if (!(condition)) {abort()};
#else
    #define CHECK(condition)
#endif

struct buf_t
{
    char* buffer;
    size_t capacity;
    size_t size;
};

struct buf_t *buf_new(size_t capacity)
{
    char* tmp = (char*) malloc(capacity);

    if (tmp == NULL)
        return NULL;

    buf_t* buf = (buf_t*) malloc(sizeof(buf_t));

    if (buf == NULL)
    {
        free(tmp);
        return NULL;
    }

    buf -> buffer = tmp;
    buf -> capacity = capacity;
    buf -> size = 0;

    return buf;
}

void buf_free(struct buf_t *buf)
{
    CHECK(buf != NULL);
    free(buf);
}

size_t buf_capacity(struct buf_t *buf)
{
    CHECK(buf != NULL);
    return buf -> capacity;
}

size_t buf_size(struct buf_t *buf)
{
    CHECK(buf != NULL);
    return buf -> size;
}

ssize_t buf_fill(fd_t fd, buf_t *buf, size_t required)
{
    CHECK(buf != NULL);
    CHECK(required <= buf -> capacity);

    if (buf -> size >= required)
        return buf -> size;

    int have = 0;

    while (1)
    {
        have = read(fd, buf -> buffer + buf -> size, buf -> capacity - buf -> size);

        if (have == 0)
            return buf -> size;
        else if (have == -1)
            return -1;
        else if (buf -> size + have >= required)
            return buf -> size += have;

        buf -> size += have;
    }
}

ssize_t buf_flush(fd_t fd, buf_t *buf, size_t required)
{
    CHECK(buf != NULL);
    CHECK(required <= buf -> capacity);

    int have = 0;
    int res = 0;

    while (1)
    {
        have = write(fd, buf -> buffer, buf -> size);

        if (have == 0)
            return res;
        if (have == -1)
            return -1;

        memmove(buf -> buffer, buf -> buffer + have, buf -> size - have);
        buf -> size -= have;
        res += have;

        if (res >= required)
            return res;
    }
}
