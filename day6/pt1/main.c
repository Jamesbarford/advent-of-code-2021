/**
 * This is a very naive basic solution
 */
#include <stdio.h>

#include "../../includes/readfile.h"
#include "../../includes/xmalloc.h"

struct listNode;
typedef struct listNode {
    int value;
    struct listNode *next;
} listNode;

typedef struct list {
    listNode *head;
    unsigned long size;
} list;

listNode *listNodeNew(int value) {
    listNode *ln;
    ln = xmalloc(sizeof(listNode));
    ln->next = NULL;
    ln->value = value;
    return ln;
}

list *listNew(void) {
    list *l;
    l = xmalloc(sizeof(list));
    l->head = listNodeNew(0);
    l->size = 0;
    return l;
}

void listAdd(list *l, int value) {
    listNode *ln;
    ln = listNodeNew(value);
    ln->next = l->head;
    l->head = ln;
    l->size++;
}

void listRelease(list *l) {
    listNode *ln, *next;
    ln = l->head;

    while (ln) {
        next = ln->next;
        xfree(ln);
        ln = next;
    }
    xfree(l);
}

list *problemParseFileToList(char *buf) {
    list *l = listNew();

    for (char *ptr = buf; *ptr != '\n' && *ptr != '\0';) { 
        switch(*ptr) {
            case ',':
                ptr++;
                break;
            default:
                listAdd(l, *(ptr++) - 48);
                break;
        }
    }
    return l;
}

void spawnFishList(list *state, unsigned int days) {
    int ready_fish = 0;
    listNode *ln;

    for (unsigned int i = 1; i <= days; ++i) {
        ln = state->head;
        while (ln) {
            ln->value--;
            if (ln->value == 0) {
                ready_fish++;
                ln->value = 7;
            }
            ln = ln->next;
        }

        if (i == days) break;

        for (; ready_fish > 0; --ready_fish)
            listAdd(state, 9);
    }
}

int main(void) {
    rFile *rf = rFileRead("../input.txt");
    list *state_list = problemParseFileToList(rf->buf);
    spawnFishList(state_list, 80);
    printf("fish: %ld\n", state_list->size);

    listRelease(state_list);
    rFileRelease(rf);
}
