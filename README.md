# Advent Of Code 2021

## Language: `c`
The aim here is speed. I've tried where possible to comment the code for some of the more bonkers lines:
E.G:
```c
/**
 * convert row to decimal
 * - find start of row: idx * (bitcount + 1)
 * - until '\n' convert to decimal
 * - shift bit to correct position
 */
for (ptr = &rf->buf[idx * (bitcount + 1)], i = bitcount; *ptr != '\n';)
	aircount[airtype] |= (*(ptr++) - 48) << --i;
```

## Build
If I've not broken it you should be able to run the following in the root of this project and it should just work(tm).

```sh
make
```

I am leaning on some things I've built, which are in the `includes` folder.