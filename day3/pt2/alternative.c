/**
 * Alternate version that feels a bit more readable
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../includes/readfile.h"
#include "../../includes/xmalloc.h"

enum AIR_TYPE {
    O2,
    CO2
};

int main(void) {
    rFile *rf;
    char *ptr;
    int i, j, k, bitcount, linecount, arraylen, ones, zeros, bit, idx;
    int *indexes, *idxToChar, *intarr;
    int aircount[2] = {0};

    rf = rFileRead("../simple.txt");
    ptr = rf->buf;

    for (bitcount = 0; *ptr != '\n'; ++bitcount, ptr++);

    ptr = rf->buf;
    linecount = rf->len / (bitcount + 1);

    indexes = xmalloc(linecount * sizeof(int));
    idxToChar = xmalloc(linecount * sizeof(int));
    intarr = xcalloc(linecount, sizeof(int));

    // Get all the 11010100's as integers
    for (i = 0; *ptr != '\0'; ++i, ptr++)
        for (j = bitcount - 1; j >= 0; --j)
            intarr[i] |= (*(ptr++) - 48) << j;

    for (enum AIR_TYPE airtype = O2; airtype < 2; airtype++) {
        arraylen = linecount;

        for (i = 0; i < linecount; ++i) {
            indexes[i] = i;
            idxToChar[i] = 0;
        }

        for (i = bitcount - 1; i >= 0 ; --i) {
            for (k = 0, ones = 0, zeros = 0; k < linecount; ++k) {
                bit = (intarr[k] >> i) & 1;
                if (indexes[k] == -1) continue;
                if (bit == 1) ones++;
                else zeros++;
                idxToChar[k] = bit;
            }

            for (k = 0; k < linecount; ++k) {
                if (indexes[k] == -1) continue;
                if (airtype == O2 ? ones >= zeros : zeros > ones) {
                    if (idxToChar[k] == 1) continue;
                    indexes[k] = -1;
                    arraylen--;
                } else {
                    if (idxToChar[k] == 0) continue;
                    indexes[k] = -1;
                    arraylen--;
                }
            }
            if (arraylen == 1) break;
        }

        for (i = 0, idx = -1; i < linecount; ++i) {
            if ((idx = indexes[i]) != -1) {
                aircount[airtype] = intarr[idx];
                break;
            }
        }
    }
    
    printf("\n");
    printf("oxygen: %d\n", aircount[O2]);
    printf("co2: %d\n", aircount[CO2]);
    printf("Answer: %d\n", aircount[O2] * aircount[CO2]);

    rFileRelease(rf);
    xfree(indexes);
    xfree(idxToChar);
    xfree(intarr);
}
