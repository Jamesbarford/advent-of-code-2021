#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../includes/readfile.h"
#include "../../includes/xmalloc.h"

#define X1 0
#define Y1 1
#define X2 2
#define Y2 3

/* This should be big enough */
#define VECTOR_SIZ 1 << 16

#define isnumber(n) ((n) >= 48 && (n) <= 57)
#define ascii2int(n, c) ((n) = (n)*10 + (c - 48))

enum LineDir {
    HORIZONTAL,
    VERTICAL,
    DIAGONAL
};

typedef struct point {
    int coords[4];
} point;

typedef struct vector {
    unsigned int len;
    unsigned int capacity;
    point *entries;
} vector;

#define vectorGet(v, i) ((v)->entries[(i)].coords)
#define vectorGetCoord(v, i, c) (vectorGet((v), (i))[(c)])
#define vectorSetCoord(v, i, c, n) (vectorGet((v), (i))[(c)] = (n))
#define vectorUnsetCoords(v, i)                                                \
    (vectorGet((v), (i))[X1] = vectorGet((v), (i))[Y1] =                       \
         vectorGet((v), (i))[X2] = vectorGet((v), (i))[Y2] = 0)

vector *vectorNew(unsigned int capacity) {
    vector *v;
    v = xmalloc(sizeof(vector));
    v->len = 0;
    v->capacity = capacity;
    v->entries = xmalloc(sizeof(point) * v->capacity);
    return v;
}

void vectorPrint(vector *v) {
    printf("length: %u\n", v->len);
    printf("capacity: %u\n", v->capacity);
    printf("[\n");
    for (unsigned int i = 0; i < v->len; ++i) {
        if (i + 1 != v->len)
            printf("  (%d,%d -> %d,%d),\n", vectorGetCoord(v, i, X1),
                   vectorGetCoord(v, i, Y1), vectorGetCoord(v, i, X2),
                   vectorGetCoord(v, i, Y2));
        else
            printf("  (%d,%d -> %d,%d)", vectorGetCoord(v, i, X1),
                   vectorGetCoord(v, i, Y1), vectorGetCoord(v, i, X2),
                   vectorGetCoord(v, i, Y2));
    }
    printf("\n]\n");
}

void vectorRelease(vector *v) {
    if (v) {
        free(v->entries);
        free(v);
    }
}

void setMaxBounds(vector *v, int *maxY, int *maxX) {
    int x1, y1, x2, y2;

    // increment by one as 0-9 = 10 spaces not 9 for the canvas;
    x1 = vectorGetCoord(v, v->len, X1) + 1;
    y1 = vectorGetCoord(v, v->len, Y1) + 1;
    x2 = vectorGetCoord(v, v->len, X2) + 1;
    y2 = vectorGetCoord(v, v->len, Y2) + 1;

    if (x1 > *maxX)  *maxX = x1;
    else if (x2 > *maxX) *maxX = x2;

    if (y1 > *maxY) *maxY = y1;
    else if (y2 > *maxY) *maxY = y2;
}

vector *problemParseFile(char *buf, int *maxY, int *maxX) {
    vector *v;
    int coord, curnum;
    char *ptr;

    ptr = buf;
    v = vectorNew(VECTOR_SIZ);
    coord = 0;

    while (*ptr != '\0') {
        switch (*ptr) {
        case ',':
            coord++;
            ptr++;
            break;
        case ' ':
            coord++;
            ptr += 4;
            break;
        case '\n':
            coord = 0;
            ptr++;
            setMaxBounds(v, maxY, maxX);
            v->len++;
            break;
        default:
            /* build our number, silence gcc warning by splitting*/
            curnum = ascii2int(vectorGetCoord(v, v->len, coord), *ptr);
            vectorSetCoord(v, v->len, coord, curnum);
            ptr++;
            break;
        }
    }

    return v;
}

void canvasPrint(int *canvas, int maxY, int maxX) {
    for (int y = 0; y < maxY; ++y) {
        for (int x = 0; x < maxX; ++x) {
            int val = canvas[x + y * maxY];
            if (val == 0)
                printf(".");
            else
                printf("%d", val);
        }
        printf("\n");
    }
}

void genericPlot(int x1, int y1, int x2, int y2, int maxY, int *canvas) {
    enum LineDir dir;

    if (x1 == x2) dir = VERTICAL;
    else if (y1 == y2) dir = HORIZONTAL;
    else dir = DIAGONAL;

    switch (dir) {
        case VERTICAL:
            if (y1 < y2) {
                for (; y1 <= y2; ++y1) {
                    canvas[x1 + y1 * maxY]++;
                }
            } else {
                for (; y1 >= y2; --y1) {
                    canvas[x1 + y1 * maxY]++;
                }
            }
            break;

        case HORIZONTAL:
            if (x1 < x2) {
                for (; x1 <= x2; ++x1) {
                    canvas[x1 + y1 * maxY]++;
                }
            } else {
                for (; x1 >= x2; --x1) {
                    canvas[x1 + y1 * maxY]++;
                }
            }
            break;

        /* this is gross */
        case DIAGONAL:
            if (x1 <= x2 && y1 <= y2) {
                for (; x1 <= x2 && y1 <= y2; ++y1, ++x1)
                     canvas[x1 + y1 * maxY]++;

            } else if (x1 >= x2 && y1 >= y2) {
                for (; x1 >= x2 && y1 >= y2; --y1, --x1)
                     canvas[x1 + y1 * maxY]++;

            } else if (x1 >= x2 && y1 <= y2) {
                for (; x1 >= x2 && y1 <= y2; ++y1, --x1)
                     canvas[x1 + y1 * maxY]++;

            } else {
                for (; x1 <= x2 && y1 >= y2; --y1, ++x1)
                     canvas[x1 + y1 * maxY]++;
            }
            break;
        default:
            fprintf(stderr, "Invalid direction: %d\n", dir);
            exit(EXIT_FAILURE);
    }
}

void canvasPlot(vector *v, int *canvas, int maxY) {
    int x1, y1, x2, y2;
    x1 = y1 = x2 = y2 = 0;

    for (unsigned int i = 0; i < v->len; ++i) {
        x1 = vectorGetCoord(v, i, X1);
        x2 = vectorGetCoord(v, i, X2);
        y1 = vectorGetCoord(v, i, Y1);
        y2 = vectorGetCoord(v, i, Y2);

        genericPlot(x1, y1, x2, y2, maxY, canvas);
    }
}

int main(void) {
    int maxX, maxY, acc;
    int *canvas;

    maxX = -100;
    maxY = -100;

    rFile *rf = rFileRead("../input.txt");
    vector *v = problemParseFile(rf->buf, &maxY, &maxX);
    canvas = xcalloc(maxX * maxY, sizeof(int));
    acc = 0;

    canvasPlot(v, canvas, maxY);
    canvasPrint(canvas, maxY, maxX);

    for (int i = 0; i < maxX * maxY; ++i)
        if (canvas[i] >= 2)
            acc++;

    printf("%d\n", acc);
    vectorRelease(v);
    rFileRelease(rf);
}
