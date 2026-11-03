all:
	$(MAKE) -C ./part1;
	$(MAKE) -C ./part2;
	$(MAKE) -C ./part3;
clean:
	$(MAKE) ./part1 clean;
	$(MAKE) ./part2 clean;
	$(MAKE) ./part3 clean;
