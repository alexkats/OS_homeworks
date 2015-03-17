all:
	$(MAKE) -C cat
	$(MAKE) -C revwords

clean:
	$(MAKE) -C cat clean
	$(MAKE) -C revwords clean
