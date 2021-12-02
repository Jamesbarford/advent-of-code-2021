#include <stdio.h>

#include "../../includes/readfile.h"

enum DIR {
    Depth,
    Horizontal,
    Aim
};

#define isnumber(c) ((c) >= 48 && (c) <= 57)

int main(void) {
    rFile *rf;
    char *ptr;
    enum DIR dir; 
    int ht[3] = {0};
    int sign;

    rf = rFileRead("../input.txt");
    ptr = rf->buf;
    sign = 1;
    dir = Horizontal;

    while (*ptr != '\0') {
        int res = 0;
        /* we dont need to parse the whole string as the
         * first character is unique */
        if (*ptr == 'f') {
            sign = 1;
            dir = Horizontal;
        } else if (*ptr == 'd') {
            sign = 1;
            dir = Aim;
        } else if (*ptr == 'u') {
            sign = -1;
            dir = Aim;
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
        switch (dir) {
            case Aim: {
                ht[Aim] += (sign * res);
                break;
            }
            case Horizontal: {
                ht[Horizontal] += res;
                ht[Depth] += (res * ht[Aim]);
                break;
            }
            default:
                fprintf(stderr, "Invalid direction\n");
                break;

        }

        /* advance */
        ptr++;
    }

    printf("%d\n", ht[Depth] * ht[Horizontal]);
}