all: forking polling
	rm -f forking.o
	rm -f polling.o
	
libs:
	$(MAKE) -C ../lib

forking: libs
	gcc -Wall -std=c99 -o forking forking.c -I../lib -Wl,-rpath=../lib -L../lib -lbufio

polling:
	gcc -Wall -std=c99 -o polling polling.c -I../lib -Wl,-rpath=../lib -L../lib -lbufio

clean:
	rm -f forking.o
	rm -f forking
	rm -f polling.o
	rm -f polling
	$(MAKE) -C ../lib clean
