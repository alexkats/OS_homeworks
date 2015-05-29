all: cat
	
libs:
	$(MAKE) -C ../lib

cat: libs
	gcc -Wall -std=c99 -o cat cat.c -I../lib -Wl,-rpath=../lib -L../lib -lhelpers

clean:
	rm -f cat.o
	rm -f cat
	rm -f cat2.c
	$(MAKE) -C ../lib clean
