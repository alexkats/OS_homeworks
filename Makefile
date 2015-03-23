all:
	$(MAKE) -C cat
	$(MAKE) -C revwords
	$(MAKE) -C lenwords

clean:
	$(MAKE) -C cat clean
	$(MAKE) -C revwords clean
	$(MAKE) -C lenwords clean
