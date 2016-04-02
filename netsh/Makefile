all: netsh
	rm -f netsh.o
	
libs:
	$(MAKE) -C ../lib

netsh: libs
	g++ -Wall -std=c++11 -o netsh netsh.cpp -I../lib -Wl,-rpath=../lib -L../lib -lbufio

clean:
	rm -f netsh.o
	rm -f netsh
	$(MAKE) -C ../lib clean
