#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../includes/readfile.h"
#include "../includes/xmalloc.h"

#define SETCAPACITY (1<<10)
#define VECSIZ      (1<<10) // more than big enough

#define abs(x) ((x) < 0 ? (x) * -1 : (x))
#define toint(p) ((p)-48)

struct setEntry;
typedef struct setEntry {
    long value;
    struct setEntry *next;
} setEntry;

typedef struct set {
    int len;
    long mask;
    setEntry **entries;
} set;

setEntry *setEntryNew(long value) {
    setEntry *se;
    se = xmalloc(sizeof(setEntry));
    se->value = value;
    se->next = NULL;
    return se;
}

set *setNew(void) {
    set *s = xmalloc(sizeof(set));
    s->len = 0;
    s->entries = xcalloc(SETCAPACITY, sizeof(long));
    s->mask = SETCAPACITY - 1;
    return s;
}

long hash(int x, int y) {
    return (long)x | (long)y << 32L;
}

void unhash(long value, int *i1, int *i2) {
    *i1 = (long)value & 0x7FFFFFFFL;
    *i2 = (long)value >> 32L & 0x7FFFFFFFL;
}

setEntry *setFind(set *s, long value) {
    for (setEntry *se = s->entries[s->mask & value]; se != NULL; se = se->next)
        if (se->value == value)
            return se;
    return NULL;
}

int setContains(set *s, long value) {
    return setFind(s, value) != NULL;
} 

int setAdd(set *s, long value) {
    if (setContains(s, value)) return 0;
    setEntry *se = setEntryNew(value);
    long idx = s->mask & value;

    se->next = s->entries[idx];
    s->entries[idx] = se;
    s->len++;
    return 1;
}

void setPrint(set *s) {
    int x, y;
    setEntry *se;

    printf("{\n");
    for (int i = 0; i < SETCAPACITY; ++i) {
        if ((se = s->entries[i]) != NULL) {
            while (se) { 
                unhash(se->value, &x, &y);
                printf("  [%ld] => x:%d, y:%d\n", se->value, x, y);
                se = se->next;
            }

        }
    }
    printf("}\n");
}

void setReleaseEntries(set *s) {
    for (int i = 0; i < SETCAPACITY; ++i) {
        if (s->entries[i] != NULL) {
            setEntry *se, *next;

            se = s->entries[i];
            if (se->next) {
                while (se) {
                    next = se->next;
                    xfree(se);
                    se = next;
                }
            }
            xfree(se);
        }
    }
    s->len = 0;
}

void setRelease(set *s) {
    setReleaseEntries(s);
    xfree(s);
}

typedef struct vector {
    int len;
    int capacity;
    unsigned long *entries;
} vector;

vector *vectorNew(void) {
    vector *v;
    v = xmalloc(sizeof(vector));
    v->len = 0;
    v->capacity = VECSIZ;
    v->entries = xcalloc(v->capacity, sizeof(unsigned long));
    return v;
}

void vectorRelease(vector *v) {
    xfree(v->entries);
    xfree(v);
}

void vectorPush(vector *v, unsigned long num) {
    v->entries[v->len++] = num;
}

void vectorPrint(vector *v) {
    printf("[\n");
    int x,y;
    for (int i = 0; i < v->len; ++i) {
        long value = v->entries[i];
        unhash(value, &x, &y);
        printf("  [%d]: x:%3d, y:%3d\n", i, x, y);
    }
    printf("]\n");
}

vector *vectorUniq(vector *v) {
    unsigned long *newEntries, *oldEntries;
    int i,j;
    set *unique;

    oldEntries = v->entries;
    unique =setNew();
    newEntries = xcalloc(VECSIZ, sizeof(long));

    for (i = 0, j = 0; i < v->len; ++i)
        if (setAdd(unique, oldEntries[i]))
            newEntries[j++] = oldEntries[i];

    setReleaseEntries(unique);
    v->entries = newEntries;
    v->len = j;
    xfree(oldEntries);
    return v;
}

typedef struct paperDimensions {
    int x;
    int y;
} paperDimensions;

void vectorGetPaperDimensions(vector *v, paperDimensions *dims) {
    int x,y;

    dims->x = -10000;
    dims->y = -10000;

    for (int i = 0; i < v->len; ++i) {
        unhash(v->entries[i], &x, &y);
        if (x > dims->x) dims->x = x;
        if (y > dims->y) dims->y = y;
    }
}

void problemParseFile(char *buf, vector *folds, vector *manual) {
    int x=0,y=0;
    int isY = 0;
    char *ptr = buf;

    while (1) {
        switch (*ptr) {
            case ',':
                isY = 1;
                ptr++;
                break;
            case '\n':
                isY = 0;
                vectorPush(manual, hash(x, y));
                if (*(ptr + 1) == '\n') goto parsefolds;
                x = y = 0;
                ptr++;
                break;
            default:
                if (isY)
                    y = y * 10 + (*(ptr++) - 48);
                else
                    x = x * 10 + (*(ptr++) - 48);
                break;
        }
    }

parsefolds:
    ptr++;

    while (*ptr != '\0') {
        if (*ptr == 'f') {
            ptr += 11;
            char axis = *ptr;
            int pos = 0;
            ptr += 2;
            while (*ptr != '\n')
                pos = pos * 10 + (*(ptr++) - 48);
            vectorPush(folds, hash(axis, pos));
        }
        ptr++;
    }
}

void printManual(vector *manual, paperDimensions *dims) {
    int maxX = dims->x;
    int maxY = dims->y;
    long rep_size = (maxX) * (maxY) * sizeof(char);
    char *visual_representation = xmalloc(rep_size);
    int xx,yy;

    memset(visual_representation, ' ', rep_size);

    for (int i = 0; i < manual->len; ++i) {
        unhash(manual->entries[i],&xx,&yy);
        visual_representation[xx + yy * maxX] = '#';
    }

    for (int y = 0; y < maxY; ++y) {
        for (int x = 0; x < maxX; ++x) {
            printf("%c", visual_representation[x + y * maxX]);
        }
        printf("\n");
    }

    xfree(visual_representation);
}

void foldY(vector *v, int foldline) {
    for (int i = 0, x, y; i < v->len; ++i) {
        unhash(v->entries[i], &x, &y);
        if (y > foldline)
            v->entries[i] = (long)hash(x, foldline * 2 - y);
    }
}

void foldX(vector *v, int foldline) {
    for (int i = 0, x, y; i < v->len; ++i) {
        unhash(v->entries[i], &x, &y);
        if (x > foldline)
            v->entries[i] = (long)hash(foldline * 2 - x, y);
    }
}

int solveProblemTwo(vector *manual, vector *folds) {
    int foldline, axis;
    paperDimensions dims;

    vectorGetPaperDimensions(manual, &dims);

    for (int i = 0; i < folds->len; ++i) {
        unhash(folds->entries[i], &axis, &foldline);
        if (axis == 'y') {
            foldY(manual, foldline);
            dims.y = foldline;
        } else {
            foldX(manual, foldline);
            dims.x = foldline;
        }
    }
    manual = vectorUniq(manual);
    printManual(manual, &dims);

    return manual->len;
}

int solveProblemOne(vector *manual, vector *folds) {
    int foldline, axis;
    paperDimensions dims;

    vectorGetPaperDimensions(manual, &dims);

    for (int i = 0; i < folds->len; ++i) {
        unhash(folds->entries[i], &axis, &foldline);
        if (axis == 'y') foldY(manual, foldline);
        else foldX(manual, foldline);
        break;
    }

    manual = vectorUniq(manual);

    return manual->len;
}


int main(int argc, char **argv) {
    rFile *rf = rFileRead("./input.txt");
    vector *manual = vectorNew();
    vector *folds = vectorNew();

    problemParseFile(rf->buf, folds, manual);

    if (argc == 1) {
        fprintf(stderr, "Usage: %s <c1|c2>\n"
                "c1: challenge 1 and c2: challenge2\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (strncmp(argv[1], "c1", 2) == 0) {
        printf("challenge1: %d\n", solveProblemOne(manual, folds));
    } else if (strncmp(argv[1], "c2", 2) == 0) {
        printf("challenge2: %d\n", solveProblemTwo(manual, folds));
    } else {
        fprintf(stderr, "Invalid option: \"%s\" valid options are c1 or c2",
                argv[1]);
        exit(EXIT_FAILURE);
    }

    vectorRelease(manual);
    vectorRelease(folds);
    rFileRelease(rf);
}
