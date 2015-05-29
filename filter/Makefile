all: filter
	
libs:
	$(MAKE) -C ../lib

filter: libs
	gcc -Wall -std=c99 -o filter filter.c -I../lib -Wl,-rpath=../lib -L../lib -lhelpers

clean:
	rm -f filter.o
	rm -f filter
	$(MAKE) -C ../lib clean
