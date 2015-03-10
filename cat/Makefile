CC=gcc
CFLAGS=-Wall -I../lib 
LDFLAGS=-L../lib -lhelpers
PROJECT=cat

all:
	$(PROJECT)

$(PROJECT):
	$(CC) -o $@ $(LDFLAGS) ../lib/libhelpers.so

$(PROJECT).o:
	$(CC) -o $@ -c $(CFLAGS) $(PROJECT).c

../lib/libhelpers.so:
	$(MAKE) -C ../lib

clean:
	rm -f $(PROJECT).o
	rm -f $(PROJECT)
