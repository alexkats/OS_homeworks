CC=gcc
CFLAGS=-Wall -I../lib -std=c99
LDFLAGS=-L../lib
LIBS=../lib/libhelpers.so
PROJECT=filter

all: $(PROJECT)
#	install_name_tool -change libhelpers.so ../lib/libhelpers.so ./filter
	rm -f $(PROJECT).o

libhelpers.so:
	$(MAKE) -C ../lib

$(PROJECT).o:
	$(CC) -o $@ -c $(CFLAGS) $(PROJECT).c

$(PROJECT): $(PROJECT).o libhelpers.so
	$(CC) -o $@ $(LDFLAGS) $< $(LIBS)

clean:
	rm -f $(PROJECT).o
	rm -f $(PROJECT)
	rm -f /tmp/filter.tar
	$(MAKE) -C ../lib clean
