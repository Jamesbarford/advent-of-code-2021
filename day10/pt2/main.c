/**
 * I could use the vector for both, but I had the linked list anyway
 */
#include <stdio.h>
#include <stdlib.h>

#include "../../includes/readfile.h"
#include "../../includes/xmalloc.h"

#define OK  1
#define ERR 0

#define VECSIZ (1<<16) // more than big enough

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

void vectorPush(vector *v, unsigned long num) {
    v->entries[v->len++] = num;
}

int intcmp(const void *i1, const void *i2) {
    return *(unsigned long *)i1 > *(unsigned long *)i2;
}

/* Sort from lowest to highest */
void vectorSort(vector *v) {
    qsort(v->entries, v->len, sizeof(unsigned long), intcmp);
}

void vectorPrint(vector *v) {
    printf("[");
    for (int i = 0; i < v->len; ++i) {
        printf("  %ld,\n", v->entries[i]);
    }
    printf("]\n");
}

struct listEntry;
typedef struct listEntry {
    char symbol;
    struct listEntry *next;
} listEntry;

typedef struct list {
    int size;
    listEntry *head;
} list;

listEntry *listEntryNew(char symbol) {
    listEntry *le;
    le = xmalloc(sizeof(listEntry));
    le->next = NULL;
    le->symbol = symbol;
    return le;
}

list *listNew(void) {
    list *l;
    l = xmalloc(sizeof(list));
    l->size = 0;
    return l;
}

void listAdd(list *l, char symbol) {
    listEntry *le = listEntryNew(symbol);
    le->next = l->head;
    l->head = le;
    l->size++;
}

listEntry *listPop(list *l) {
    if (l->size == 0) return NULL;
    listEntry *tmp;
    tmp = l->head;
    if (l->head->next)
        l->head = l->head->next;
    else
        l->head = NULL;
    l->size--;
    return tmp;
}

void listPrint(list *l) {
    listEntry *le = l->head;
    while (le) {
        printf("'%c' ", le->symbol);
        le = le->next;
    }
    printf("\n");
}

void listReleaseEntries(list *l) {
    listEntry *le = l->head, *next;

    while (le) {
        next = le->next;
        xfree(le);
        le = next;
    }
    l->head = NULL;
    l->size = 0;
}

void listRelease(list *l) {
    listReleaseEntries(l);
    xfree(l);
}

int handleList(list *l, char symbol) {
    listEntry *le;

    switch (symbol) {
        case '(':
        case '[':
        case '{':
        case '<':
            listAdd(l, symbol);
            break;
        case ')':
            le = listPop(l);
            if (le->symbol != '(') return ERR;
            break;
        case ']':
            le = listPop(l);
            if (le->symbol != '[') return ERR;
            break;
        case '>':
            le = listPop(l);
            if (le->symbol != '<') return ERR;
            break;
        case '}':
            le = listPop(l);
            if (le->symbol != '{') return ERR;
            break;
    }
    return OK;
}

unsigned long getMissingBraceScore(list *l) {
    unsigned long acc = 0;
    listEntry *le;

    while ((le = listPop(l)) != NULL) {
        acc *= 5;
        switch (le->symbol) {
            case '(': acc += 1; break;
            case '[': acc += 2; break;
            case '{': acc += 3; break;
            case '<': acc += 4; break;
        }
        xfree(le);
    }

    return acc;
}

vector *solveProblemTwo(char *buf) {
    int is_row_err = 0;
    unsigned long result;
    vector *v = vectorNew();
    list *l = listNew();

    for (char *ptr = buf; *ptr != '\0'; ptr++) {
        if (*ptr == '\n') {
            if (!is_row_err) {
                result = getMissingBraceScore(l);
                vectorPush(v, result);
            }
            listReleaseEntries(l);
            is_row_err = 0;
        }
        if (handleList(l, *ptr) == ERR) is_row_err = 1;
    }
    return v;
}

int main(void) {
    rFile *rf = rFileRead("../input.txt");
    vector *scores = solveProblemTwo(rf->buf);
    vectorSort(scores);

    printf("%ld\n", scores->entries[scores->len/2]);

    rFileRelease(rf);
}
