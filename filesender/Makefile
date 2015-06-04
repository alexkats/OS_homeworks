all: filesender
	rm -f filesender.o
	
libs:
	$(MAKE) -C ../lib

filesender: libs
	gcc -Wall -std=c99 -o filesender filesender.c -I../lib -Wl,-rpath=../lib -L../lib -lbufio

create:
	echo "Hello, World!" > hello.txt
	dd if=/dev/zero of=zeros.bin count=1000000

clean:
	rm -f filesender.o
	rm -f filesender
	rm -f hello.txt
	rm -f zeros.bin
	$(MAKE) -C ../lib clean
