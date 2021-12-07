#include <stdio.h>

#include "../../includes/readfile.h"
#include "../../includes/xmalloc.h"

#define VECSIZ (1<<16)

typedef struct vector {
    unsigned long len;
    unsigned long capacity;
    unsigned long *entries;
} vector;

#define vectorGet(v, i) ((v)->entries[(i)])
#define vectorSet(v, i, n) ((v)->entries[(i)] = (n))

vector *vectorNew(void) {
    vector *v;
    v = xmalloc(sizeof(vector));
    v->len = 0;
    v->capacity = VECSIZ;
    v->entries = xmalloc(v->capacity * sizeof(unsigned long));
    return v;
} 

void vectorRelease(vector *v) {
    xfree(v->entries);
    xfree(v);
}

void vectorPush(vector *v, unsigned long val) {
    if (v->len * 4 >= v->capacity * 3) {
        unsigned long newcapacity = v->capacity << 1;
        unsigned long *oldentries = v->entries;
        unsigned long *newentries = xmalloc(sizeof(unsigned long) *
                newcapacity);
        for (unsigned long i = 0; i < v->len; ++i)
            newentries[i] = oldentries[i];
        v->capacity = newcapacity;
    }
    v->entries[v->len++] = val;
}

vector *parseFileToVector(char *buf) {
    vector *v = vectorNew();
    unsigned long num = 0;

    for (char *ptr = buf; *ptr != '\0';) {
        switch (*ptr) {
            case ',':
                ptr++;
                vectorPush(v, num);
                num = 0;
                break;
            case '\n':
                vectorPush(v, num);
                return v;
            default:
                num = num * 10 + (*(ptr++) - 48);
                break;
        }
    }
    return v;
}

unsigned long helpCrabsAlign(vector *v) {
    unsigned long destination, curpos, low, acc;
    low = 10000000;

    for (unsigned long i = 0; i < v->len; ++i) {
        acc = 0;
        destination = vectorGet(v, i);

        for (unsigned long j = 0; j < v->len; ++j) {
            curpos = vectorGet(v, j);

            if (curpos > destination) {
                acc += curpos - destination;
            } else if (curpos < destination) {
                acc += destination - curpos;
            } else { // ==
                continue;
            }
        }
        if (acc < low) low = acc;
    }

    return low;
}

int main(void) {
    rFile *rf = rFileRead("../input.txt");
    vector *v = parseFileToVector(rf->buf); 
    unsigned long fuelEfficientPosition = helpCrabsAlign(v);

    printf("Fuel cost: %ld\n", fuelEfficientPosition);

    rFileRelease(rf);
}
