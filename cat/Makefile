CC=gcc
CFLAGS=-Wall -I../lib 
LDFLAGS=-L../lib -lhelpers
PROJECT=cat

all: $(PROJECT)

$(PROJECT): $(PROJECT).o libhelpers.so
	$(CC) -o $@ $(LDFLAGS) $<

$(PROJECT).o:
	$(CC) -o $@ -c $(CFLAGS) $(PROJECT).c

libhelpers.so:
	$(MAKE) -C ../lib

clean:
	rm -f $(PROJECT).o
	rm -f $(PROJECT)
