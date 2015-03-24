all:
	$(MAKE) -C cat
	$(MAKE) -C revwords
	$(MAKE) -C filter

clean:
	$(MAKE) -C cat clean
	$(MAKE) -C revwords clean
	$(MAKE) -C filter clean
