all: bufcat
	
libs:
	$(MAKE) -C ../lib

bufcat: libs
	gcc -Wall -std=c99 -o bufcat bufcat.c -I../lib -Wl,-rpath=../lib -L../lib -lbufio

clean:
	rm -f bufcat.o
	rm -f bufcat
	rm -f bufcat2.c
	$(MAKE) -C ../lib clean
