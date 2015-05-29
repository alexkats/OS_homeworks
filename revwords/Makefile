all: revwords
	
libs:
	$(MAKE) -C ../lib

revwords: libs
	gcc -Wall -std=c99 -o revwords revwords.c -I../lib -Wl,-rpath=../lib -L../lib -lhelpers

clean:
	rm -f revwords.o
	rm -f revwords
	$(MAKE) -C ../lib clean
