default: all

.DEFAULT:
	cd includes && $(MAKE) $@
	cc ./template/createday.c -o createday -O2 -Wall -Werror -Wextra
