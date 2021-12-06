/**
 * this solves both pt1 + pt2
 *
 * The linked list implementation in pt1 required to much memory and
 * crashed my computer
 */
#include <stdio.h>

#include "../includes/readfile.h"
#include "../includes/xmalloc.h"

#define DICTSIZ 9

unsigned long problemParseFile(char *buf, unsigned long *entries) {
    unsigned long count = 0;

    for (char *ptr = buf; *ptr != '\n' && *ptr != '\0';) {
        switch(*ptr) {
            case ',':
                ptr++;
                break;
            default:
                entries[*(ptr++) - 48]++;
                count++;
                break;
        }
    }

    return count;
}

unsigned long spawnFish(unsigned long *entries, unsigned int days,
        unsigned long count)
{
    unsigned long e0,e1,e2,e3,e4,e5,e6,e7,e8;

    for (unsigned int i = 1; i <= days; ++i) {
        e0 = entries[0];
        e1 = entries[1];
        e2 = entries[2];
        e3 = entries[3];
        e4 = entries[4];
        e5 = entries[5];
        e6 = entries[6];
        e7 = entries[7];
        e8 = entries[8];

        entries[0] = e1;
        entries[1] = e2;
        entries[2] = e3;
        entries[3] = e4;
        entries[4] = e5;
        entries[5] = e6;
        entries[6] = e7 + e0;
        entries[7] = e8;
        entries[8] = e0;
        count += e0;
    }

    return count;
}

int main(void) {
    rFile *rf = rFileRead("./input.txt");
    unsigned long entries[DICTSIZ] = {0ul};
    unsigned long count = problemParseFile(rf->buf, entries);
    count = spawnFish(entries, 256, count);

    printf("size: %ld\n", count);

    rFileRelease(rf);
}
