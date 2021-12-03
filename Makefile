default: all

.DEFAULT:
	cc ./template/createday.c -o createday -O2 -Wall -Werror -Wextra
	@cd includes && $(MAKE) $@
	# this is going to get ridiculous
	@cd ./day1/pt1 && $(MAKE) $@
	@cd ./day1/pt2 && $(MAKE) $@
	@cd ./day2/pt1 && $(MAKE) $@
	@cd ./day2/pt2 && $(MAKE) $@
	@cd ./day3/pt1 && $(MAKE) $@
	@cd ./day3/pt2 && $(MAKE) $@

clean:
	rm ./createday
	rm ./includes/*.o
	rm ./day1/pt1/*.out
	rm ./day1/pt2/*.out
	rm ./day2/pt1/*.out
	rm ./day2/pt2/*.out
	rm ./day3/pt1/*.out
	rm ./day3/pt2/*.out
