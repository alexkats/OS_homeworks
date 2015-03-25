CC=gcc
CFLAGS=-Wall -I../lib
LDFLAGS=-L../lib -lhelpers
PROJECT=revwords

all: $(PROJECT)
#	install_name_tool -change libhelpers.so ../lib/libhelpers.so ./revwords
	rm -f $(PROJECT).o

libhelpers.so:
	$(MAKE) -C ../lib

$(PROJECT).o:
	$(CC) -o $@ -c $(CFLAGS) $(PROJECT).c

$(PROJECT): $(PROJECT).o libhelpers.so
	$(CC) -o $@ $(LDFLAGS) $<

clean:
	rm -f $(PROJECT).o
	rm -f $(PROJECT)
	$(MAKE) -C ../lib clean
