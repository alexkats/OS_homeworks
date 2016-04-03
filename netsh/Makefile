all: netsh
	rm -f netsh.o
	
libs:
	$(MAKE) -C ../cpplib

netsh: libs
	g++ -Wall -std=c++11 -o netsh netsh.cpp -I../cpplib -Wl,-rpath=../cpplib -L../cpplib -lhelpers -lbufio

clean:
	rm -f netsh.o
	rm -f netsh
	rm -f log
	rm -f /tmp/netsh.log
	$(MAKE) -C ../cpplib clean
