#include <stdio.h>

#include "../../includes/readfile.h"

enum DIR {
    Depth,
    Horizontal
};

#define isnumber(c) ((c) >= 48 && (c) <= 57)

int main(void) {
    rFile *rf;
    char *ptr;
    enum DIR dir;
    int ht[2] = {0};
    int sign;

    rf = rFileRead("../input.txt");
    ptr = rf->buf;
    dir = Depth;
    sign = 1;

    while (*ptr != '\0') {
        int res = 0;
        /* we dont need to parse the whole string as the
         * first character is unique */
        if (*ptr == 'f') {
            sign = 1;
            dir = Horizontal;
        } else if (*ptr == 'd') {
            sign = 1;
            dir = Depth;
        } else if (*ptr == 'u') {
            sign = -1;
            dir = Depth;
        }

        /* move to first digit */
        while (!isnumber(*ptr))
            ptr++;

        /* create number from string */
        while(*ptr != '\n') {
            res = res * 10 + *ptr - 48;
            ptr++;
        }

        /* accumulate */
        ht[dir] += (sign * res);

        ptr++;
    }

    printf("%d\n", ht[Depth] * ht[Horizontal]);
}