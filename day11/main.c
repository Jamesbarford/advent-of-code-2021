#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../includes/readfile.h"
#include "../includes/xmalloc.h"

#define toint(n) ((n) - 48)
#define SETCAPACITY (1<<5)


unsigned int getlineLength(char *buf) {
    unsigned int len = 0;
    char *ptr = buf;

    while (*ptr != '\n') {
        ptr++;
        len++;
    }

    return len;
}

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
    s->entries = xcalloc(1 << 5, sizeof(long));
    s->mask = SETCAPACITY - 1;
    return s;
}

long hash(int x, int y) {
    long hash = 0;
    hash |= x;
    hash |= (long)y << 32L;
    return hash;
}

void unhash(long value, int *x, int *y) {
    *x = (long)value & 0x7FFFFFFF;
    *y = (long)value >> 32L & 0x7FFFFFFF;
}

setEntry *setFind(set *s, long value) {
    setEntry *se = s->entries[s->mask & value];
    while (se) {
        if (se->value == value)
            return se;
        se = se->next;
    }
    return NULL;
}

int setContains(set *s, long value) {
    return setFind(s, value) != NULL;
} 

void setAdd(set *s, long value) {
    if (setContains(s, value)) return;
    setEntry *se = setEntryNew(value);
    long idx = s->mask & value;

    se->next = s->entries[idx];
    s->entries[idx] = se;
    s->len++;
}

void setPrint(set *s) {
    long value;
    int x, y;
    printf("{\n");
    for (int i = 0; i < SETCAPACITY; ++i) {
        if (s->entries[i] != NULL) {
            value = s->entries[i]->value;
            unhash(value, &x, &y);
            printf("  [%ld] => x:%d, y:%d\n", value, x, y);
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

typedef struct matrix {
    char rows;
    char columns;
    char *entries;
} matrix;

matrix *matrixNew(int rows, int columns) {
    matrix *m;
    m = xmalloc(sizeof(matrix));
    m->rows = rows;
    m->columns = columns;
    m->entries = xmalloc(sizeof(char) * (rows * columns));
    return m;
}

int matrixBoundsCheck(matrix *m , int x, int y) {
    return (y <= m->rows -1 && y >= 0) && (x <= m->columns -1 && x >= 0);
}

int matrixGet(matrix *m, int x, int y) {
    if (!matrixBoundsCheck(m, x, y)) return -1;
    return m->entries[x + y * m->columns];
}

/* easier for this to handle bounds checking */
void matrixSet(matrix *m, int x, int y, int n) {
    if (!matrixBoundsCheck(m, x, y)) return;
    m->entries[x + y * m->columns] = n;
}

void matrixRelease(matrix *m) {
    xfree(m->entries);
    xfree(m);
}

void matrixCompactPrint(matrix *m) {
    for (int y = 0; y < m->rows; ++y) {
        for (int x = 0; x < m->columns; ++x)
            printf("%d", m->entries[x + y * m->columns]);
        printf("\n");
    }
    printf("\n");
}

matrix *problemToMatrix(char *buf, unsigned int len) {
    unsigned int linelen = getlineLength(buf);
    unsigned int rowcount = len/(linelen + 1);
    matrix *m = matrixNew(rowcount, linelen);
    int currow = 0, curcol = 0;
    char *ptr = buf;

    while (1) {
        switch (*ptr) {
            case '\0': return m;
            case '\n':
                ptr++;
                currow++;
                curcol = 0;
                break;
            default:
                matrixSet(m, curcol, currow, toint(*ptr));
                ptr++;
                curcol++;
                break;
        }
    }

    return m;
}

void handleOctopusFlash(matrix *m, set *s, int x, int y);

// Flashing does not increment a 0
void matrixHandleFlash(matrix *m, set *s, int x, int y) {
    int val = matrixGet(m, x, y);
    long _hash = hash(x,y);

    if (setContains(s, _hash)) return;
    if (val == -1) return;
    if (val > 9) {
        handleOctopusFlash(m, s, x, y);
        matrixSet(m, x, y, 0);
        setAdd(s, _hash);
        return;
    } else {
        matrixSet(m, x, y, val + 1);
        if (val + 1 > 9)
            handleOctopusFlash(m, s, x, y);
    }
}

// diagonals, up down & left right
void handleOctopusFlash(matrix *m, set *s, int x, int y) {
    if (setContains(s, hash(x,y))) return;
    if (matrixGet(m, x, y) > 9) {
        setAdd(s, hash(x, y));
        matrixSet(m, x, y, 0);
        matrixHandleFlash(m, s, x + 1, y);
        matrixHandleFlash(m, s, x - 1, y);

        matrixHandleFlash(m, s, x, y + 1);
        matrixHandleFlash(m, s, x, y - 1);

        matrixHandleFlash(m, s, x + 1, y + 1);
        matrixHandleFlash(m, s, x + 1, y - 1);

        matrixHandleFlash(m, s, x - 1, y + 1);
        matrixHandleFlash(m, s, x - 1, y - 1);
    }
}

void matrixAddOne(matrix *m) {
    for (int x = 0; x < m->rows; ++x) {
        for (int y = 0; y < m->columns; ++y) {
            m->entries[x + y  * m->columns]++;
        }
    }
}

unsigned long matrixScanForFlashingOctopuses(matrix *m) {
    set *s = setNew();
    unsigned long setlen;

    for (int y = 0; y < m->columns; ++y) {
        for (int x = 0; x < m->rows; ++x) {
            if (m->entries[x + y * m->columns] > 9) {
                handleOctopusFlash(m, s, x, y);
            }
        }
    }

    setlen = s->len;
    setReleaseEntries(s);
    return setlen;
}

unsigned long solveProblemOne(matrix *m, int iters) {
    unsigned long acc = 0;
    for (int i = 0; i < iters; ++i) {
        matrixAddOne(m);
        acc += matrixScanForFlashingOctopuses(m);
    }
    return acc;
}

unsigned long solveProblemTwo(matrix *m) {
    unsigned long numflashes = 0;
    unsigned long iter = 1;

    while(1) {
        matrixAddOne(m);
        numflashes = matrixScanForFlashingOctopuses(m);

        if (numflashes == (unsigned long)m->rows * m->columns) return iter;
        iter++;
    }
}

int main(int argc, char **argv) {
    char *progname = argv[0];
    rFile *rf = rFileRead("./input.txt");
    matrix *m = problemToMatrix(rf->buf, rf->len);
    
    if (argc == 1) {
        fprintf(stderr, "Usage: %s <c1|c2>\n"
                "c1: challenge 1 and c2: challenge2\n", progname);
        exit(EXIT_FAILURE);
    }

    if (strncmp(argv[1], "c2", 2) == 0) {
        printf("challenge2: %ld\n", solveProblemTwo(m));
    } else if (strncmp(argv[1], "c1", 2) == 0) {
        printf("challenge1: %ld\n", solveProblemOne(m, 200));
    } else {
        fprintf(stderr, "Invalid option: \"%s\" valid options are c1 or c2",
                argv[1]);
        exit(EXIT_FAILURE);
    }

    matrixRelease(m);
    rFileRelease(rf);
}
