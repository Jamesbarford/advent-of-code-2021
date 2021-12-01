/**
 * Version 1.0 October 2021 - read in entire file
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
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "readfile.h"
#include "xmalloc.h"

/**
 * read line into outbuf, outbuf must be big enough
 * to contain a line, up to max in size
 */
int readline(char **ptr, char *outbuf, int max) {
    int i = 0;
    while (**ptr != '\n' && **ptr != '\0' && i++ < max)
        *outbuf++ = *(*ptr)++;
    *outbuf = '\0';
    if (**ptr == '\0' || i > max) return 0;
    (*ptr)++;
    return 1;
}

rFile *rFileRead(char *filename) {
    int fd;
    off_t filesize;
    rFile *rf;

    rf = xmalloc(sizeof(rFile));

    if ((fd = open(filename, O_RDWR, 0666)) == -1) {
        fprintf(stderr, "Failed to open(2) '%s' %s\n", filename,
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    if ((filesize = lseek(fd, 0, SEEK_END)) == -1) {
        fprintf(stderr, "Failed to lseek(2) %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    rf->buf = xmalloc(sizeof(char) * filesize + 1);
    rf->len = filesize;

    if (lseek(fd, 0, SEEK_SET) == -1) {
        fprintf(stderr, "Failed to lseek(2) %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (read(fd, rf->buf, rf->len) != rf->len) {
        fprintf(stderr, "Failed to read(2) %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    rf->buf[rf->len] = '\0';

    close(fd);

    return rf;
}

void rFileRelease(rFile *rf) {
    xfree(rf->buf);
    xfree(rf);
}

#ifdef TEST_READFILE
int main(int argc, char **argv) {
    rFile *rf;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    rf = rFileRead(argv[1]);
    printf("%s\n", rf->buf);
}
#endif
