CC=gcc
CFLAGS=-Wall -I../lib -std=c99
LDFLAGS=-L../lib
LIBS=../lib/libhelpers.so -lhelpers ../lib/libbufio.so -lbufio
PROJECT=bufcat

all: $(PROJECT)
#	install_name_tool -change libhelpers.so ../lib/libhelpers.so ./bufcat
#	install_name_tool -change libbufio.so ../lib/libbufio.so ./bufcat
	rm -f $(PROJECT).o

libs:
	$(MAKE) -C ../lib

$(PROJECT).o:
	$(CC) -o $@ -c $(CFLAGS) $(PROJECT).c

$(PROJECT): $(PROJECT).o libs
	$(CC) -o $@ $(LDFLAGS) $< $(LIBS)

clean:
	rm -f $(PROJECT).o
	rm -f $(PROJECT)
	rm -f $(PROJECT)2.c
	$(MAKE) -C ../lib clean
