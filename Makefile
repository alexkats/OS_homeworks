all:
	$(MAKE) -C cat
	$(MAKE) -C revwords
	$(MAKE) -C filter
	$(MAKE) -C bufcat
	$(MAKE) -C simplesh
	$(MAKE) -C filesender
	$(MAKE) -C bipiper

clean:
	$(MAKE) -C cat clean
	$(MAKE) -C revwords clean
	$(MAKE) -C filter clean
	$(MAKE) -C bufcat clean
	$(MAKE) -C simplesh clean
	$(MAKE) -C filesender clean
	$(MAKE) -C bipiper clean
