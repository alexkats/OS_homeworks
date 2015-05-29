all:
	$(MAKE) -C cat
	$(MAKE) -C revwords
	$(MAKE) -C filter
	$(MAKE) -C bufcat
	$(MAKE) -C buffilter
	$(MAKE) -C simplesh

clean:
	$(MAKE) -C cat clean
	$(MAKE) -C revwords clean
	$(MAKE) -C filter clean
	$(MAKE) -C bufcat clean
	$(MAKE) -C buffilter clean
	$(MAKE) -C simplesh clean
