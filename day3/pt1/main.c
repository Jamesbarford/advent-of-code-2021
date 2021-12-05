#include <stdio.h>
#include <stdlib.h>

#include "../../includes/readfile.h"
#include "../../includes/xmalloc.h"

int main(void) {
    rFile *rf;
    char *ptr;
    int *arr, i, j, gamma, epsilon, bitcount;

    rf =  rFileRead("../input.txt");
    gamma = 0;
    epsilon = 0;
    ptr = rf->buf;

    // find how many bits we have in our line
    for (bitcount = 0; *ptr != '\n'; ++bitcount, ptr++);

    /* we can now make the array */
    arr = xcalloc(sizeof(int), bitcount);

    for (ptr = rf->buf; *ptr != '\0'; ptr++) {
        i = 0;
        while (*ptr != '\n') {
            /* a positive number will mean '1' was most common
             * negative '0' most common */
            arr[i++] += *(ptr++) == '1' ? 1 : -1;
        }
    }

    /* convert findings to a decimal shifting a 1 or 0
     * to the correct position */
    for (i = bitcount - 1, j = 0; i >= 0; --i, ++j) {
        gamma |= (arr[j] > 0 ? 1 : 0) << i;
        epsilon |= (arr[j] > 0 ? 0 : 1) << i;
    }

    printf("%d\n", gamma * epsilon);
    rFileRelease(rf);
    free(arr);
}

