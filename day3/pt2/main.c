/**
 * This is a bit of a shambles.
 *
 * As I don't want to store `n` number of duped strings, I'm hacking around
 * using integer arrays, marking something as deleted with `-1` then
 * skipping that array on an interation.
 *
 * EDIT: I decided to use calculate o2 and co2 in parallel using unix processes,
 * this is probably a bit much
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "../../includes/readfile.h"
#include "../../includes/xmalloc.h"

enum AIR_TYPE {
    O2,
    CO2
};

/* to get an int from one process to another we need to use buffers */
void packInt64(unsigned char *buf, long long i) {
    *buf++ = i >> 56;
    *buf++ = i >> 48;
    *buf++ = i >> 40;
    *buf++ = i >> 32;
    *buf++ = i >> 24;
    *buf++ = i >> 16;
    *buf++ = i >> 8;
    *buf++ = i;
}

unsigned long long unpackUInt64(unsigned char *buf) {
    return
        ((unsigned long long)buf[0] << 56) |
        ((unsigned long long)buf[1] << 48) |
        ((unsigned long long)buf[2] << 40) |
        ((unsigned long long)buf[3] << 32) |
        ((unsigned long long)buf[4] << 24) |
        ((unsigned long long)buf[5] << 16) |
        ((unsigned long long)buf[6] <<  8) |
        buf[7];
}

unsigned int calculateAirRating(char *buf, int linecount, int bitcount,
        int linelen, enum AIR_TYPE airtype)
{
    int j, k, idx, i;
    int ones, zeros;
    int arraylen = linecount;
    char *ptr;
    int *indexes, *idxToChar;
    unsigned int aircount;

    indexes = xmalloc(linecount * sizeof(int));
    idxToChar = xmalloc(linecount * sizeof(int));
    aircount = 0;

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
            if (buf[i + j] == '1') ones++;
            else zeros++;

            /* store first character, so we know what to delete */
            idxToChar[k] = buf[i + j];
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
    for (ptr = &buf[idx * (bitcount + 1)], i = bitcount; *ptr != '\n';)
        aircount |= (*(ptr++) - 48) << --i;

    xfree(indexes);
    xfree(idxToChar);

    return aircount;
}

/**
 * Use 1 process for o2 and one for co2
 */
void getsolutionWithFork(char *buf, int linecount, int linelen, int bitcount) {
    unsigned char msg[9] = {'\0'};
    unsigned int o2rating, co2rating;
    pid_t cpid;
    int pipefd[2];

    if (pipe(pipefd) == -1) {
        fprintf(stderr, "Failed to pipe(2) %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if ((cpid = fork()) == -1) {
        fprintf(stderr, "Failed to fork(2) %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* child handles oxygen */
    if (cpid == 0) { // child handles writes
        close(pipefd[0]);
        o2rating = calculateAirRating(buf, linecount,
                bitcount, linelen, O2);
        /* pack the uInt into a buffer for writting */
        packInt64(msg, o2rating);
        if (write(pipefd[1], msg, 9) <= 0) {
            fprintf(stderr, "Failed to write(2) to parent %s\n",
                    strerror(errno));
            exit(EXIT_FAILURE);
        }
        _exit(EXIT_SUCCESS);
    } else {
        close(pipefd[1]);
        co2rating = calculateAirRating(buf, linecount,
                bitcount, linelen, CO2);
        wait(NULL);
        if ((read(pipefd[0], msg, 9)) <= 0) {
            fprintf(stderr, "Failed to read(2) from child %s\n",
                    strerror(errno));
            exit(EXIT_FAILURE);
        }

        /* get the uInt from the buffer */
        o2rating = unpackUInt64(msg);
        printf("%u, %u\n", co2rating, o2rating);
        close(pipefd[0]);
        printf("Answer: %u\n", o2rating * co2rating);
    }
}

int main(void) {
    rFile *rf;
    char *ptr;
    int bitcount, linecount, linelen;

    rf = rFileRead("../input.txt");
    linecount = 0;
    ptr = rf->buf;

    // find how many bits we have in our line
    for (bitcount = 0; *ptr != '\n'; ++bitcount, ptr++);

    // number of chars / line length
    linecount = rf->len / (bitcount + 1);
    linelen = bitcount + 1; // account for '\n';

    getsolutionWithFork(rf->buf, linecount, linelen, bitcount);
    exit(EXIT_SUCCESS);
}

