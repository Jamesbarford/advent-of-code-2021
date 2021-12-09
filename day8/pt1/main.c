#include <stdio.h>

#include "../../includes/readfile.h"

#define isdeliminator(p) ((p) == ' ' || (p) == '\n' || (p) == '\0')

unsigned int getOccurances(char *buf) {
    char *ptr = buf;
    int run = 0;
    unsigned int acc = 0;

    while (1) {
        while (*ptr != '|')
            ptr++;
        ptr += 2;
        while (*ptr != '\n' && *ptr != '\0') {
            run++;
            ptr++;
            if (isdeliminator(*ptr)) {
                if (run == 2 || run == 3 || run == 4 || run == 7) {
                    acc++;
                }
                run = -1;
            }
        }
        run = 0;
        if (*ptr == '\0' || *(ptr + 1) == '\0') break;
        ptr++;
    }

    return acc;
}

int main(void) {
    rFile *rf = rFileRead("../input.txt");
    unsigned int answer = getOccurances(rf->buf);

    printf("answer: %u\n", answer);

    rFileRelease(rf);
}
