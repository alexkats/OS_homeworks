all: simplesh
	
libs:
	$(MAKE) -C ../lib

simplesh: libs
	gcc -Wall -std=c99 -o simplesh simplesh.c -I../lib -Wl,-rpath=../lib -L../lib -lhelpers -lbufio

clean:
	rm -f simplesh
	$(MAKE) -C ../lib clean
