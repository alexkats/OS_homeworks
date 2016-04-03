CC=g++
CFLAGS=-Wall -fPIC -std=c++11

all: libhelpers.so libbufio.so
	rm -f helpers.o
	rm -f bufio.o

libhelpers.so: helpers.o
	$(CC) -o $@ -shared $<

libbufio.so: bufio.o
	$(CC) -o $@ -shared $<

helpers.o:
	$(CC) -o $@ -c $(CFLAGS) helpers.cpp

bufio.o:
	$(CC) -o $@ -c $(CFLAGS) bufio.cpp

clean:
	rm -f libhelpers.so
	rm -f helpers.o
	rm -f libbufio.so
	rm -f bufio.o
