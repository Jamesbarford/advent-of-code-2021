 /**
 * Version 1.0 October 2021 - hexdump utils
 *
 * Copyright (c) 2021, James Barford-Evans
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>

#include "hexdump.h"

void hexdump(unsigned char *data, unsigned int len) {
    unsigned char byte;
    unsigned int i, j;

    for (i = 0; i < len; ++i) {
        byte = data[i];

        printf("%02x ", byte);

        if (((i % 16) == 15) || (i == len - 1)) {
            for (j = 0; j < 15 - (i % 16); ++j)
                printf("   ");
            printf("| ");
            for (j = (i - (i % 16)); j <= i; ++j) {
                byte = data[j];
                if ((byte > 31) && (byte < 127))
                    printf("%c", byte);
                else
                    printf(".");
            }
            printf("\n");
        }
    }
}

static inline void printBinary(unsigned long num) {
    for (unsigned long i = 0; i < 32; i++) {
        if ((num >> (unsigned long)(31ul - i)) & 1ul) {
            printf("1");
        } else {
            printf("0");
        }
    }
    printf("\n");
}

static inline void printCharBinary(char c) {
    for (int i = 0; i < 4; ++i) {
        if ((c >> (3u - i)) & 1u)
            printf("1");
        else
            printf("0");
    }
    printf("\n");
}

void binarydump(void *data, unsigned int len) {
    for (unsigned int i = 0; i < len; ++i) {
        printCharBinary(((char *)data)[i]);
    }
    printf("\n");
}

void printBits(void *ptr, unsigned int len) {
    unsigned char *b = (unsigned char *)ptr;
    unsigned char byte;
    int i, j;

    for (i = len - 1; i >= 0; i--) {
        for (j = 7; j >= 0; j--) {
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
        printf("\n");
    }
    printf("\n");
}
