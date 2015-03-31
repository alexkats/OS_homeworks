CC=gcc
CFLAGS=-Wall -fPIC -std=c99

all: libhelpers.so
	rm -f helpers.o

libhelpers.so: helpers.o
	$(CC) -o $@ -shared $<

helpers.o:
	$(CC) -o $@ -c $(CFLAGS) helpers.c

clean:
	rm -f libhelpers.so
	rm -f helpers.o
