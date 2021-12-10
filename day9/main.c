/**
 * Note to self: USE this implementation of a set for the other challenges
 */
#include <stdio.h>
#include <stdlib.h>

#include "../includes/xmalloc.h"
#include "../includes/readfile.h"

#define toint(p) ((p) - 48)
#define DICTSIZ 10 // numbers are 0-9
#define SETSIZ (1<<10)
#define VECSIZ (1<<16)

enum CoordType {
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT,
    EDGE_LEFT,
    EDGE_RIGHT,
    TOP_ROW,
    BOTTOM_ROW,
    MIDDLE, // anything that has four surrounding numbers
};

typedef struct vector {
    int len;
    int capacity;
    int *entries;
} vector;

#define vectorGet(v, i) ((v)->entries[(i)])
#define vectorSet(v, i, n) ((v)->entries[(i)] = (n))

vector *vectorNew(void) {
    vector *v;
    v = xmalloc(sizeof(vector));
    v->len = 0;
    v->capacity = VECSIZ;
    v->entries = xcalloc(v->capacity, sizeof(int));
    return v;
} 

void vectorRelease(vector *v) {
    xfree(v->entries);
    xfree(v);
}

void vectorPush(vector *v, int val) {
    if (v->len * 4 >= v->capacity * 3) {
        int newcapacity = v->capacity << 1;
        int *oldentries = v->entries;
        int *newentries = xmalloc(sizeof(int) * newcapacity);
        for (int i = 0; i < v->len; ++i)
            newentries[i] = oldentries[i];
        v->capacity = newcapacity;
    }
    v->entries[v->len++] = val;
}

int vectorPop(vector *v) {
    if (v->len == 0) return -1;
    int retval = v->entries[v->len-1];
    v->entries[v->len-1] = -1;
    v->len--;
    return retval;
}

int intcmp(const void *i1, const void *i2) {
    return *(int *)i1 - *(int *)i2;
}

void vectorSort(vector *v) {
    qsort(v->entries, v->len, sizeof(int), intcmp);
}

void vectorPrint(vector *v) {
    printf("[");
    for (int i = 0; i < v->len; ++i) {
        if (i + 1 == v->len)
            printf("%d", v->entries[i]);
        else
            printf("%d, ", v->entries[i]);
    }
    printf("]\n");
}

typedef struct dictInt {
    int entries[DICTSIZ];
} dictInt;

void dictIntAdd(dictInt *d, int val) {
    d->entries[val]++;
}

dictInt *dictIntNew(void) {
    dictInt *d;
    d = xmalloc(sizeof(dictInt));
    for (int i = 0; i < DICTSIZ; ++i)
        d->entries[i] = 0;
    return d;
}

void dictIntRelease(dictInt *d) {
    xfree(d);
}

void dictIntPrint(dictInt *d) {
    for (int i = 0; i < DICTSIZ; ++i)
        printf("[%d] => %d\n", i, d->entries[i]);
}

unsigned int dictIntAccumulate(dictInt *d) {
    unsigned int acc = 0;
    unsigned int cur;

    for (int i = 0; i < DICTSIZ; ++i) {
        cur = d->entries[i] != 0 ? (i + 1) * d->entries[i] : 0;
        acc += cur;
    }

    return acc;
}


unsigned int getlineLength(char *buf) {
    unsigned int len = 0;
    char *ptr = buf;

    while (*ptr != '\n') {
        ptr++;
        len++;
    }

    return len;
}

int getIndexOfSmallest(int *arr, int len) {
    int min = 10000;
    int idx = -1;
    for (int i = 0; i < len; ++i)
        if (arr[i] < min) {
            min = arr[i];
            idx = i;
        }
    return idx;
}

typedef struct matrix {
    char rows;
    char columns;
    char *entries;
} matrix;

#define matrixGetIdx(m, x, y) ((x) + (y) * (m)->columns)
#define matrixGet(m, x, y)    ((m)->entries[matrixGetIdx((m), (x), (y))])
#define matrixSet(m, x, y, n) ((m)->entries[matrixGetIdx((m), (x), (y))] = (n))

matrix *matrixNew(int rows, int columns) {
    matrix *m;
    m = xmalloc(sizeof(matrix));
    m->rows = rows;
    m->columns = columns;
    m->entries = xmalloc(sizeof(char) * (rows * columns));
    return m;
}

void matrixRelease(matrix *m) {
    xfree(m->entries);
    xfree(m);
}

void matrixPrint(matrix *m) {
    printf("[\n");
    int val;

    for (int y = 0; y < m->rows; ++y) {
        printf("  [");
        for (int x = 0; x < m->columns; ++x) {
            val = matrixGet(m, x, y);
            if (x + 1 != m->columns)
                printf("%d, ", val);
            else
                printf("%d", val);
        }
        if (y + 1 != m->rows)
            printf("],\n");
        else
            printf("]\n");
    }
    printf("]\n");
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

enum CoordType getCoord(matrix *m, int x, int y) {
    if (x == 0 && y == 0) return TOP_LEFT;
    if (x == m->columns-1 && y == 0) return TOP_RIGHT;
    if (y == 0) return TOP_ROW;
    if (x == 0 && y == m->rows-1) return BOTTOM_LEFT;
    if (x == m->columns-1 && y == m->rows-1) return BOTTOM_RIGHT;
    if (y == m->rows-1) return BOTTOM_ROW;
    if (x == 0) return EDGE_LEFT;
    if (x == m->columns-1) return EDGE_RIGHT;
    return MIDDLE;
}

/**
 * This will determine if the number we are looking at is the smallest in 
 * comparison to its neighbours. It's very tedious.
 */
int isSmallestIdx(matrix *m, enum CoordType coord, int x, int y) {
    int tmparray[5];
    int current = matrixGet(m, x, y);

    int left   = (x - 1) + y * m->columns;
    int right  = (x + 1) + y * m->columns;
    int top    = x + (y - 1) * m->columns;
    int bottom = x + (y + 1) * m->columns;

    switch (coord) {
        case TOP_LEFT: {
            tmparray[0] = m->entries[right];
            tmparray[1] = m->entries[bottom];
            tmparray[2] = current;

            return getIndexOfSmallest(tmparray, 3) == 2;
        }

        case TOP_ROW: {
            tmparray[0] = m->entries[bottom];
            tmparray[1] = m->entries[right];
            tmparray[2] = m->entries[left];
            tmparray[3] = current;

            return getIndexOfSmallest(tmparray, 4) == 3;
        }

        case TOP_RIGHT: {
            tmparray[0] = m->entries[left];
            tmparray[1] = m->entries[bottom];
            tmparray[2] = current;

            return getIndexOfSmallest(tmparray, 3) == 2;
        }

        case EDGE_LEFT: {
            tmparray[0] = m->entries[right];
            tmparray[1] = m->entries[bottom];
            tmparray[2] = m->entries[top];
            tmparray[3] = current;

            return getIndexOfSmallest(tmparray, 4) == 3;
        }

        case MIDDLE: {
            tmparray[0] = m->entries[left];
            tmparray[1] = m->entries[right];
            tmparray[2] = m->entries[bottom];
            tmparray[3] = m->entries[top];
            tmparray[4] = current;

            return getIndexOfSmallest(tmparray, 5) == 4;
        }

        case EDGE_RIGHT: {
            tmparray[0] = m->entries[left];
            tmparray[1] = m->entries[bottom];
            tmparray[2] = m->entries[top];
            tmparray[3] = current;

            return getIndexOfSmallest(tmparray, 4) == 3;
        }

        case BOTTOM_LEFT: {
            tmparray[0] = m->entries[top];
            tmparray[1] = m->entries[right];
            tmparray[2] = current;

            return getIndexOfSmallest(tmparray, 3) == 2;
        }

        case BOTTOM_ROW: {
            tmparray[0] = m->entries[right];
            tmparray[1] = m->entries[top];
            tmparray[2] = m->entries[left];
            tmparray[3] = current;

            return getIndexOfSmallest(tmparray, 4) == 3;
        }

        case BOTTOM_RIGHT: {
            tmparray[0] = m->entries[top];
            tmparray[1] = m->entries[left];
            tmparray[2] = current;
        
            return getIndexOfSmallest(tmparray, 3) == 2;
        }

        default:
            fprintf(stderr, "Invalid CoordType: %d\n", coord);
            exit(EXIT_FAILURE);
    }
}

dictInt *solveProblemOne(matrix *m) {
    int val, issmallest;
    dictInt *d = dictIntNew();
    enum CoordType coord;

    for (int y = 0; y < m->rows; ++y) {
        for (int x = 0; x < m->columns; ++x) {
            val = matrixGet(m, x, y);
            coord = getCoord(m, x, y);
            issmallest = isSmallestIdx(m, coord, x, y);
            if (issmallest) {
                dictIntAdd(d, val);
            }
        }
    }
    
    return d;
}

void floodfill(char x, char y, matrix *m, int *acc) {
    if (x < 0 || x >= m->columns) return;
    if (y < 0 || y >= m->rows) return;
    if (matrixGet(m, x, y) == 9) return;
    matrixSet(m, x, y, 9);
    (*acc)++;
    floodfill(x + 1, y, m, acc);
    floodfill(x - 1, y, m, acc);
    floodfill(x, y + 1, m, acc);
    floodfill(x, y - 1, m, acc);
}

vector *solveProblemTwo(matrix *m) {
    int issmallest;
    vector *v = vectorNew();
    enum CoordType coord;
    int acc;

    for (int y = 0; y < m->rows; ++y) {
        for (int x = 0; x < m->columns; ++x) {
            coord = getCoord(m, x, y);
            issmallest = isSmallestIdx(m, coord, x, y);
            if (issmallest) {
                acc = 0;
                floodfill(x, y, m, &acc);
                vectorPush(v,acc);
            }
        }
    }
    vectorSort(v);
    return v;
}

int main(void) {
    rFile *rf = rFileRead("./input.txt");
    matrix *m = problemToMatrix(rf->buf, rf->len);
    dictInt *d = solveProblemOne(m);
    vector *v = solveProblemTwo(m);

    printf("challenge1: %u\n", dictIntAccumulate(d));

    vectorPrint(v);
    printf("challenge2: %d\n",
            v->entries[v->len-1] *
            v->entries[v->len-2] *
            v->entries[v->len-3]);

    matrixRelease(m);
    rFileRelease(rf);
}
