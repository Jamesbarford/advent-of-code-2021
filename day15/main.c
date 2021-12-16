#include <stdio.h>

#include "../includes/readfile.h"
#include "../includes/xmalloc.h"

#define toint(p) ((p)-48)

enum Compass {
    NORTH,
    SOUTH,
    EAST,
    WEST,
};

typedef struct matrix {
    int rows;
    int columns;
    int *entries;
} matrix;

matrix *matrixNew(int rows, int columns) {
    matrix *m;
    m = xmalloc(sizeof(matrix));
    m->rows = rows;
    m->columns = columns;
    m->entries = xcalloc(m->rows * m->columns, sizeof(int));
    return m;
}

#define matrixGet(m, x, y)                                                     \
    (((y) <= (m)->rows - 1 && (y) >= 0) && ((x) <= m->columns - 1 && (x) >= 0) \
                    ? ((m)->entries[(x) + (y) * (m)->columns])                 \
                    : -1)

#define matrixSet(m, x, y, n)                                                  \
    (((y) <= (m)->rows - 1 && (y) >= 0) && ((x) <= m->columns - 1 && (x) >= 0) \
            ? ((m)->entries[(x) + (y) * (m)->columns] = (n)),                  \
            1 : 0)

#define matrixHas(m, x, y)                                                     \
    (((y) <= (m)->rows - 1 && (y) >= 0) && ((x) <= m->columns - 1 && (x) >= 0) \
                    ? 1                                                        \
                    : 0)

void matrixRelease(matrix *m) {
    xfree(m->entries);
    xfree(m);
}

void matrixprint(matrix *m) {
    for (int y = 0; y < m->rows; ++y) {
        for (int x = 0; x < m->columns; ++x)
            printf("%d", m->entries[x + y * m->columns]);
        printf("\n");
    }
    printf("\n");
}

matrix *problemToMatrix(char *buf, int len) {
    matrix *m;
    char *ptr;
    int linelen = 0, rows = 0, currow = 0, curcol = 0;

    for (ptr = buf; *ptr != '\n'; ptr++)
        linelen++;

    rows = len / (linelen + 1);
    m = matrixNew(rows, linelen);
    ptr = buf;

    while (1) {
        switch (*ptr) {
        case '\0':
            return m;
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

int getSmallestIdx(int *array, int len) {
    int min = 10000000, i = 0;
    for (; i < len; ++i)
        if (array[i] != -1 && array[i] < min)
            min = i;
    return i;
}

typedef struct heapNode {
    int weight;
    int x;
    int y;
} heapNode;

heapNode *heapNodeNew(int weight, int x, int y) {
    heapNode *hn;
    hn = xmalloc(sizeof(heapNode));
    hn->weight = weight;
    hn->x = x;
    hn->y = y;
    return hn;
}

typedef int heapNodeCmp(heapNode *hn1, heapNode *hn2);

// this is the implementation of a simple priority queue
typedef struct heap {
    int size;
    int capacity;
    heapNodeCmp *compare;
    heapNode **entries;
} heap;

int weightcmp(heapNode *hn1, heapNode *hn2) {
    return hn1->weight < hn2->weight;
}

/* Treat the entries as a binary tree, despite it being a flat array */
#define heapParent(p) ((int)(((p)-1) / 2))
#define heapLeft(p) (((p)*2) + 1)
#define heapRight(p) (((p)*2) + 2)

heap *heapNew(heapNodeCmp *compare) {
    heap *hp;
    hp = xmalloc(sizeof(heap));
    hp->capacity = 1 << 30; // this should be more than big enough
    hp->entries = xcalloc(hp->capacity, sizeof(heapNode *));
    hp->size = 0;
    hp->compare = compare;
    return hp;
}

void heapNodeAssign(heapNode *hn, int weight, int x, int y) {
    hn->x = x;
    hn->y = y;
    hn->weight = weight;
}

int heapInsert(heap *hp, heapNode *new) {
    int insert_pos, parent_pos;
    heapNode *tmp;

    hp->entries[hp->size] = new;

    insert_pos = hp->size;
    parent_pos = heapParent(insert_pos);

    while (insert_pos > 0 &&
            hp->compare(hp->entries[parent_pos], hp->entries[insert_pos]))
    {
        tmp = hp->entries[parent_pos];
        hp->entries[parent_pos] = hp->entries[insert_pos];
        hp->entries[insert_pos] = tmp;

        // move up one level in the conceptual tree
        insert_pos = parent_pos;
        parent_pos = heapParent(parent_pos);
    }

    hp->size++;

    return 1;
}

heapNode *heapExtract(heap *hp) {
    if (hp->size == 0)
        return NULL;
    heapNode *hn, *save, *tmp;
    int ipos, lpos, rpos, mpos;

    hn = hp->entries[0];

    save = hp->entries[hp->size - 1];
    if (hp->size - 1 > 0)
        hp->size--;
    else {
        // we always have a heap size of 1 so there is something to compare
        // against
        return hn;
    }

    hp->entries[0] = save;

    ipos = 0;
    while (1) {
        lpos = heapLeft(ipos);
        rpos = heapRight(ipos);

        if (lpos < hp->size &&
                hp->compare(hp->entries[lpos], hp->entries[ipos])) {
            mpos = lpos;
        } else
            mpos = ipos;

        if (rpos < hp->size &&
                hp->compare(hp->entries[rpos], hp->entries[mpos]))
            mpos = rpos;

        if (mpos == ipos)
            break;
        else {
            tmp = hp->entries[mpos];
            hp->entries[mpos] = hp->entries[ipos];
            hp->entries[ipos] = tmp;
            ipos = mpos;
        }
    }
    return hn;
}

void heapPrint(heap *hp) {
    heapNode *hn;
    for (int i = 0; i < hp->size; ++i) {
        hn = hp->entries[i];
        printf("x:%d, y:%d, weight: %d\n", hn->x, hn->y, hn->weight);
    }
}

void heapRelease(heap *hp) {
    xfree(hp->entries);
    xfree(hp);
}

void getSuroundingNodes(matrix *m, int x, int y, int array[4]) {
    // we can go in 4 directions
    // we go 1 cell in each direction to determine
    // what will be the lowest cost
    array[NORTH] = matrixGet(m, x, y - 1);
    array[SOUTH] = matrixGet(m, x, y + 1);
    array[EAST] = matrixGet(m, x + 1, y);
    array[WEST] = matrixGet(m, x - 1, y);
}

int dijkstra(matrix *m) {
    int tmparray[4], total_risk, current_risk, weight, risk_matrix_value, x, y,
            shortest_path;
    heapNode *hn = NULL;
    heap *queue = heapNew(weightcmp);
    matrix *total_risk_matrix = matrixNew(m->rows, m->columns);

    heapInsert(queue, heapNodeNew(matrixGet(m, 0, 0), 0, 0));

    while (1) {
        hn = heapExtract(queue);
        x = hn->x;
        y = hn->y;
        if (queue->size > 1)
            xfree(hn);

        // we're done!
        if (x == m->columns - 1 && y == m->rows - 1)
            break;

        getSuroundingNodes(m, x, y, tmparray);
        risk_matrix_value = matrixGet(total_risk_matrix, x, y);

        for (int i = 0; i < 4; ++i) {
            int tmpy, tmpx;
            if ((weight = tmparray[i]) == -1)
                continue;
            switch (i) {
            case NORTH: {
                tmpx = x;
                tmpy = y - 1;
                break;
            }
            case SOUTH: {
                tmpx = x;
                tmpy = y + 1;
                break;
            }
            case EAST: {
                tmpx = x + 1;
                tmpy = y;
                break;
            }
            case WEST: {
                tmpx = x - 1;
                tmpy = y;
                break;
            }
            }

            total_risk = risk_matrix_value + weight;
            current_risk = matrixGet(total_risk_matrix, tmpx, tmpy);

            if (total_risk < (current_risk == 0 ? 100000 : current_risk)) {
                matrixSet(total_risk_matrix, tmpx, tmpy, total_risk);
                heapInsert(queue, heapNodeNew(total_risk, tmpx, tmpy));
            }
        }
    }
    heapRelease(queue);
    shortest_path = matrixGet(total_risk_matrix, m->columns - 1, m->rows - 1);
    matrixRelease(total_risk_matrix);
    return shortest_path;
}

int main(void) {
    rFile *rf = rFileRead("./input.txt");
    matrix *m = problemToMatrix(rf->buf, rf->len);
    matrixprint(m);
    int c1 = dijkstra(m);
    printf("c1: %d\n", c1);
}
