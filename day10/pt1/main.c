#include <stdio.h>
#include <stdlib.h>

#include "../../includes/readfile.h"
#include "../../includes/xmalloc.h"

#define OK  1
#define ERR 0

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

unsigned int solveProblemOne(char *buf) {
    list *l = listNew();
    unsigned int acc = 0;

    for (char *ptr = buf; *ptr != '\0'; ptr++) {
        if (*ptr == '\n') listReleaseEntries(l);
        if (handleList(l, *ptr) == ERR) {
            switch (*ptr) {
                case ')': acc += 3; break;
                case ']': acc += 57; break;
                case '}': acc += 1197; break;
                case '>': acc += 25137; break;
                default:
                    fprintf(stderr, "Invalid char '%c'\n", *ptr);
                    exit(EXIT_FAILURE);
            }
        }
    }
    return acc;
}

int main(void) {
    rFile *rf = rFileRead("../input.txt");

    unsigned int score = solveProblemOne(rf->buf);
    printf("%u\n", score);

    rFileRelease(rf);
}
