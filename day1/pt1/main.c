/**
 * Track number of times there was an increase
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../includes/readfile.h"

#define TMP_SIZ 100

int main(void) {
    rFile *rf;
    char *ptr, *tmpptr, tmp[TMP_SIZ];
    int cur, prev, increases;

    rf = rFileRead("./input.txt");
    ptr = rf->buf;
    tmpptr = tmp;
    prev = -1;
    cur = 0;
    increases = 0;

    while (readline(&ptr, tmpptr, sizeof(tmp))) {
        cur = atoi(tmp);

        if (prev != -1)
            if (cur > prev)
                increases++;

        memset(tmp, 0, sizeof(tmp));
        tmpptr = tmp;
        prev = cur;
    }

    printf("increases: %d\n", increases);
    rFileRelease(rf);
}
