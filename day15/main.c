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
            printf("%2d ", m->entries[x + y * m->columns]);
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

void heapNodePrint(void *_hn) {
    heapNode *hn = _hn;
    printf("x:%d, y:%d, weight:%d\n", hn->x, hn->y, hn->weight);
}

int weightcmp(void *_hn1, void *_hn2) {
    heapNode *hn1, *hn2;
    hn1 = _hn1;
    hn2 = _hn2;

    if (hn1->weight >= hn2->weight)
        return -1;
    return 1;
}

typedef struct heap {
    int size;
    // Keep it simple lets just use ints
    int (*cmp)(void *, void *);
    void (*freeValue)(void *);
    void (*print)(void *);
    void **entries;
} heap;
#define heapParent(p) ((int)(((p)-1) / 2))
#define heapLeft(p)   (((p)*2) + 1)
#define heapRight(p)  (((p)*2) + 2)

heap *heapNew(int (*cmp)(void *, void *), void (*freeValue)(void *)) {
    heap *h;
    h = xmalloc(sizeof(heap));
    h->entries = xcalloc(1 << 30, sizeof(void *));
    h->size = 0;
    h->cmp = cmp;
    h->freeValue = freeValue;
    return h;
}

void heapInsert(heap *hp, void *entry) {
    int insert_pos, parent_pos;
    void *tmp;

    hp->entries[hp->size] = entry;
    insert_pos = hp->size;
    parent_pos = heapParent(insert_pos);

    while (insert_pos > 0 &&
            hp->cmp(hp->entries[parent_pos], hp->entries[insert_pos]) < 0)
    {
        tmp = hp->entries[parent_pos];
        hp->entries[parent_pos] = hp->entries[insert_pos];
        hp->entries[insert_pos] = tmp;

        // move up one level in the conceptual tree
        insert_pos = parent_pos;
        parent_pos = heapParent(insert_pos);
    }

    hp->size++;
}

void *heapRemove(heap *hp) {
    void *hn, *save, *tmp;
    int ipos, lpos, rpos, mpos;

    hn = hp->entries[0];
    save = hp->entries[hp->size - 1];

    if ((hp->size - 1) > 0)
        hp->size--;
    else {
        hp->size=0;
        return hn;
    }

    hp->entries[0] = save;

    ipos = 0;
    lpos = heapLeft(ipos);
    rpos = heapRight(ipos);
    while (1) {
        lpos = heapLeft(ipos);
        rpos = heapRight(ipos);

        if (lpos < hp->size &&
                // this might be wrong
                hp->cmp(hp->entries[lpos], hp->entries[ipos]) > 0) {
            mpos = lpos;
        } else {
            mpos = ipos;
        }

        if (rpos < hp->size &&
                hp->cmp(hp->entries[rpos], hp->entries[mpos]) > 0)
            mpos = rpos;

        if (mpos == ipos) break;
        tmp = hp->entries[mpos];
        hp->entries[mpos] = hp->entries[ipos];
        hp->entries[ipos] = tmp;
        ipos = mpos;
    }
    return hn;
}

void heapPrint(heap *h) {
    for (int i = 0; i < h->size; ++i) {
        if (h->entries[i])
            h->print(h->entries[i]);
    }
}

void heapRelease(heap *h) {
    for (int i = 0; i < h->size; ++i) {
        if (h->freeValue)
            h->freeValue(h->entries[i]);
    }
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
    heap *queue = heapNew(weightcmp, xfree);
    queue->print = heapNodePrint;
    matrix *total_risk_matrix = matrixNew(m->rows, m->columns);

    heapInsert(queue, heapNodeNew(0, 0, 0));

    while (1) {
        hn = heapRemove(queue);
        x = hn->x;
        y = hn->y;
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

            if (total_risk < (current_risk <= 0 ? 100000 : current_risk)) {
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

int weightBigger(heapNode *hn1, heapNode *hn2) {
    if (hn1->weight >= hn2->weight)
        return 1;
    else
        return -1;
}

int main(void) {
    rFile *rf = rFileRead("./input.txt");
    matrix *m = problemToMatrix(rf->buf, rf->len);
    int c1 = dijkstra(m);
    printf("c1: %d\n", c1);
}
