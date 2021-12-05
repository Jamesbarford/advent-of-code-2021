#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../includes/readfile.h"
#include "../../includes/xmalloc.h"

#define OK 1
#define ERR 0

#define X1 0
#define Y1 1
#define X2 2
#define Y2 3

/* This should be big enough */
#define VECTOR_SIZ 1 << 16

#define isnumber(n) ((n) >= 48 && (n) <= 57)
#define ascii2int(n, c) ((n) = (n)*10 + (c - 48))

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

int coordsAreVerticalOrHorizontal(int *coords) {
    if (coords[X1] == coords[X2]) return OK;
    else if (coords[Y1] == coords[Y2]) return OK;
    return ERR;
}

void setMaxBounds(vector *v, int *maxY, int *maxX) {
    int x1, y1, x2, y2;

    // increment by one as 0-9 = 10 spaces not 9 for the canvas;
    x1 = vectorGetCoord(v, v->len, X1) + 1;
    y1 = vectorGetCoord(v, v->len, Y1) + 1;
    x2 = vectorGetCoord(v, v->len, X2) + 1;
    y2 = vectorGetCoord(v, v->len, Y2) + 1;

    if (x1 > *maxX)
        *maxX = x1;
    else if (x2 > *maxX)
        *maxX = x2;

    if (y1 > *maxY)
        *maxY = y1;
    else if (y2 > *maxY)
        *maxY = y2;
}

vector *problemParseFile(char *buf, int *maxY, int *maxX) {
    vector *v;
    int coord;
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
            /* Filter out diagonal coordinates */
            if (coordsAreVerticalOrHorizontal(vectorGet(v, v->len))) {
                setMaxBounds(v, maxY, maxX);
                v->len++;
            } else {
                /* We can re-use the same int coords[4] by setting to 0 as we've
                 * not moved */
                vectorUnsetCoords(v, v->len);
            }
            break;
        default:
            /* build our number */
            vectorSetCoord(v, v->len, coord,
                           ascii2int(vectorGetCoord(v, v->len, coord), *ptr));
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

void canvasPlot(vector *v, int *canvas, int maxY) {
    int x1, y1, x2, y2;
    x1 = y1 = x2 = y2 = 0;

    for (unsigned int i = 0; i < v->len; ++i) {
        x1 = vectorGetCoord(v, i, X1);
        x2 = vectorGetCoord(v, i, X2);
        y1 = vectorGetCoord(v, i, Y1);
        y2 = vectorGetCoord(v, i, Y2);

        if (x1 == x2) {
            if (y1 < y2) {
                for (; y1 <= y2; ++y1) {
                    canvas[x1 + y1 * maxY]++;
                }
            } else {
                for (; y1 >= y2; --y1) {
                    canvas[x1 + y1 * maxY]++;
                }
            }
        } else {
            if (x1 < x2) {
                for (; x1 <= x2; ++x1) {
                    canvas[x1 + y1 * maxY]++;
                }
            } else {
                for (; x1 >= x2; --x1) {
                    canvas[x1 + y1 * maxY]++;
                }
            }
        }
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
        if (canvas[i] > 1)
            acc++;

    printf("%d\n", acc);
    vectorRelease(v);
    rFileRelease(rf);
}
