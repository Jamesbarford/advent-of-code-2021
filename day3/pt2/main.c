/**
 * This is a bit of a shambles.
 *
 * As I don't want to store `n` number of duped strings, I'm hacking around
 * using integer arrays, marking something as deleted with `-1` then
 * skipping that array on an interation.
 *
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
    int i, bitcount, linecount, linelen;
    int *indexes, *idxToChar;
    int aircount[2] = {0};

    rf = rFileRead("../input.txt");
    linecount = 0;
    ptr = rf->buf;

    // find how many bits we have in our line
    for (bitcount = 0; *ptr != '\n'; ++bitcount, ptr++);

    // number of chars / line length
    linecount = rf->len / (bitcount + 1);
    linelen = bitcount + 1; // account for '\n';

    // ugh :(
    indexes = xmalloc(linecount * sizeof(int));
    idxToChar = xmalloc(linecount * sizeof(int));

    /* LET's Goooooo!
     * */
    for (enum AIR_TYPE airtype = O2; airtype < 2; airtype++) {
        int j, k, idx;
        int ones, zeros;
        int arraylen = linecount;

        /* setup arrays */
        for (i = 0; i < linecount; ++i) {
            indexes[i] = i;
            idxToChar[i] = 0;
        }

        for (i = 0; i < bitcount; i++) {
            ones = zeros = 0;
            /* count ones and zeros */
            for (k = 0, j = 0; j < linelen * linecount; j += linelen, k++) {
                /* this is 'deleted' so don't count this rows ones and zeros */
                if (indexes[k] == -1) continue;
                if (rf->buf[i + j] == '1') ones++;
                else zeros++;

                /* store first character, so we know what to delete */
                idxToChar[k] = rf->buf[i + j];
            }

            /* eliminate predicated on what type of air we are looking at */
            for (k = 0; k < linecount; ++k) {
                if (indexes[k] == -1)
                    continue;
                /* O2:  1's take presidence
                 * CO2: 0's take presidence */
                if (airtype == O2 ? ones >= zeros : zeros > ones) {
                    if (idxToChar[k] == '1') continue;
                    goto remove;
                } else {
                    if (idxToChar[k] == '0') continue;
                    goto remove;
                }
                remove:
                    indexes[k] = -1;
                    arraylen--;
            }

            /* we have one entry, this will be our row */
            if (arraylen == 1) break;
        }

        /* find that index */
        for (i = 0, idx = -1; i < linecount; ++i)
            if ((idx = indexes[i]) != -1)
                break;

        /**
         * convert row to decimal
         * - find start of row: idx * (bitcount + 1)
         * - until '\n' convert to decimal
         * - shift bit to correct position
         */
        for (ptr = &rf->buf[idx * (bitcount + 1)], i = bitcount; *ptr != '\n';)
            aircount[airtype] |= (*(ptr++) - 48) << --i;
    }

    printf("oxygen: %d\n", aircount[O2]);
    printf("co2: %d\n", aircount[CO2]);
    printf("Answer: %d\n", aircount[O2] * aircount[CO2]);

    rFileRelease(rf);
    xfree(indexes);
    xfree(idxToChar);
}

