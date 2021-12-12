/**
 * Think I went a bit mad creating data structures
 */
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../includes/readfile.h"
#include "../includes/xmalloc.h"

#define DICTCAPACITY (1 << 5)
#define islower(c) ((c) >= 97 && (c) <= 122)

struct listNode;
typedef struct listNode {
    void *value;
    struct listNode *next;
} listNode;

typedef struct list {
    unsigned int len;
    void (*freeValue)(void *);
    void (*print)(void *);
    listNode *head;
} list;

listNode *listNodeNew(void *value) {
    listNode *ln;
    ln = xmalloc(sizeof(listNode));
    ln->value = value;
    ln->next = NULL;
    return ln;
}

list *listNew(void) {
    list *l;
    l = xmalloc(sizeof(list));
    l->head = NULL;
    return l;
}

void listAdd(list *l, void *value) {
    listNode *ln;
    ln = listNodeNew(value);
    ln->next = l->head;
    l->head = ln;
    l->len++;
}

void *listPop(list *l) {
    listNode *ln;
    void *value;
    if ((ln = l->head) != NULL) {
        value = l->head->value;
        l->head = ln->next;
        l->len--;
        xfree(ln);
        return value;
    }
    return NULL;
}

void listRelease(void *_l) {
    list *l = _l;
    listNode *ln, *next;
    ln = l->head;

    while (ln) {
        next = ln->next;
        if (l->freeValue)
            l->freeValue(ln->value);
        xfree(ln);
        ln = next;
    }
    xfree(l);
}

void listPrint(void *_l) {
    list *l = _l;
    listNode *ln = l->head;
    printf("[");
    while (ln) {
        if (l->print) {
            l->print(ln->value);
        }
        if (ln->next)
            printf(", ");
        ln = ln->next;
    }
    printf("]\n");
}

// Daniel J. Bernstein
unsigned int hashstr(char *str) {
    unsigned int hash = (unsigned int)*str;
    for (++str; *str; ++str)
        hash = (hash << 5) - hash + (unsigned int)*str;
    return hash;
}

int keycmp(char *k1, unsigned int h1, char *k2, unsigned int h2) {
   return h1 == h2 && strcmp(k1, k2) == 0; 
}

struct dictEntry;
typedef struct dictEntry {
    char *key;
    void *value;
    unsigned int hash;
    struct dictEntry *next;
} dictEntry;

dictEntry *dictEntryNew(char *key, void *value, unsigned int hash) {
    dictEntry *de;
    de = xmalloc(sizeof(dictEntry));
    de->value = value;
    de->key = key;
    de->hash = hash;
    de->next = NULL;
    return de;
}

typedef struct dict {
    unsigned int size;
    unsigned int mask;
    void (*freeValue) (void *);
    void (*print) (void *);
    dictEntry **entries;
} dict;

dict *dictNew(void) {
    dict *d;
    d = xmalloc(sizeof(dict));
    d->size = 0;
    d->mask = DICTCAPACITY - 1;
    d->entries = xcalloc(DICTCAPACITY, sizeof(dictEntry *));
    return d;
}

dictEntry *dictFind(dict *d, char *key) {
    unsigned int hash = hashstr(key);
    dictEntry *de = d->entries[hash & d->mask];

    while (de) {
        if (keycmp(key, hash, de->key, de->hash))
            return de;
        de = de->next;
    }
    return NULL;
}

void *dictGet(dict *d, char *key) {
    dictEntry *de = dictFind(d, key);
    return de ? de->value : NULL;
}

int dictHas(dict *d, char *key) {
    return dictFind(d, key) != NULL;
}

void dictPut(dict *d, char *key, void *value) {
    dictEntry *de;
    unsigned int hash, idx;

    if ((de = dictFind(d, key)) != NULL) {
        de->value = value;
        return;
    }

    hash = hashstr(key);
    idx = hash & d->mask;
    de = dictEntryNew(key, value, hash);
    de->next = d->entries[idx];
    d->entries[idx] = de;
    d->size++;
}

void dictRemove(dict *d, char *key) {
    unsigned int hash, idx;
    dictEntry *de, *prev;

    hash = hashstr(key);
    idx = hash & d->mask;
    prev = NULL;
    de = d->entries[idx];

    while (de) {
        if (keycmp(key, hash, de->key, de->hash)) {
            if (prev) {
                prev->next = de->next;
            } else {
                d->entries[idx] = de->next;
            }
            d->size--;
            if (d->freeValue)
                d->freeValue(de->value);
            xfree(de);
            return;
        }
        prev = de;
        de = de->next;
    }
}

void dictPrint(dict *d) {
    dictEntry *de;
    for (unsigned int i = 0; i < DICTCAPACITY; ++i) {
        if ((de = d->entries[i]) != NULL) {
            while (de) {
                printf("%s => ", de->key);
                if (d->print)
                    d->print(de->value);
                de = de->next;
            }
        }
    }
}

void dictReleaseEntries(dict *d) {
    dictEntry *de, *next;
    for (unsigned int i = 0; i < DICTCAPACITY; ++i) {
        if ((de = d->entries[i]) != NULL) {
            if (de->next) {
                while (de) {
                    next = de->next;
                    if (d->freeValue) d->freeValue(de->value);
                    xfree(de);
                    de = next;
                }
            }
            xfree(de);
        }
    }
}

void dictRelease(dict *d) {
    dictReleaseEntries(d);
    xfree(d);
}

typedef struct graphNode {
    char *name;
    int islower;
} graphNode;

graphNode *graphNodeNew(void) {
    return xmalloc(sizeof(graphNode));
}

void graphNodeRelease(void *gn) {
    xfree(gn);
}

void graphNodePrint(void *_gn) {
    graphNode *gn = _gn;
    printf("%s", gn->name);
}

typedef struct graph {
    dict *d;
} graph;

graph *graphNew(void) {
    graph *g;
    g = xmalloc(sizeof(graph));
    g->d = dictNew();
    g->d->print = listPrint;
    g->d->freeValue = listRelease;
    return g;
}

list *graphListNew(void) {
    list *l = listNew();
    l->print = graphNodePrint;
    l->freeValue = graphNodeRelease;
    return l;
}

void graphPut(graph *g, char *key, char *value) {
    dictEntry *de;
    graphNode *gn;

    gn = graphNodeNew();
    gn->name = value;
    gn->islower = islower(value[0]);

    if ((de = dictFind(g->d, key)) != NULL) {
        listAdd(de->value, gn);
    } else {
        list *l = graphListNew();
        listAdd(l, gn);
        dictPut(g->d, key, l);
    }
}

// list of graph nodes
list *graphGet(graph *g, char *key) {
    return dictGet(g->d, key);
}

void graphRelease(graph *g) {
    dictRelease(g->d);
    xfree(g);
}

void graphPrint(graph *g) {
    dictPrint(g->d);
}

typedef struct set {
    dict *d;
} set;

set *setNew(void) {
    set *s;
    s = xmalloc(sizeof(set));
    s->d = dictNew();
    s->d->freeValue = NULL;
    return s;
}

int setHas(set *s, char *value) {
    return dictHas(s->d, value);
}

void setPut(set *s, char *value) {
    if (!setHas(s, value))
        dictPut(s->d, value, NULL);
}

void setRemove(set *s, char *value) {
    dictRemove(s->d, value);
}

void setReleaseEntries(set *s) {
    dictReleaseEntries(s->d);
}

void setRelease(set *s) {
    dictRelease(s->d);
    xfree(s);
}


void setPrint(set *s) {
    dict *d =s->d;
    dictEntry *de;
    printf("{");
    for (unsigned int i = 0; i < DICTCAPACITY; ++i) {
        if ((de = d->entries[i]) != NULL)
            printf("%s, ", de->key);
    }
    printf("}\n");
}

graph *problemToGraph(char *buf) {
    graph *g;
    char *keyptr, *valueptr, *ptr;
    int offset;

    offset = 0;
    ptr = &buf[offset];
    g = graphNew();

    while (1) {
        offset++;
        switch (buf[offset]) {
            case '\0': return g;
            case '-':
                buf[offset] = '\0';
                keyptr = ptr;
                offset++;
                ptr = &buf[offset];
                break;
            case '\n':
                buf[offset] = '\0';
                valueptr = ptr;

                graphPut(g, keyptr, valueptr);
                graphPut(g, valueptr, keyptr);

                offset++;
                ptr = &buf[offset];
                break;
            default:
                break;
        }
    }
}

int traverseGraphRecursivePartOne(graph *g, set *s, char *key) {
    list *l;
    graphNode *gn;
    int acc = 0, setcontains;

    if (strncmp(key, "end", 3) == 0) return 1;
    l = graphGet(g, key);
    setPut(s, key);

    for (listNode *ln = l->head; ln != NULL; ln = ln->next) {
        gn = ln->value;
        setcontains = setHas(s, gn->name);

        if (!setcontains || (!gn->islower && setcontains)) {
            acc += traverseGraphRecursivePartOne(g, s, gn->name);
            setRemove(s, gn->name);
        }
    }
    return acc;
}

int problemOneSolve(graph *g) {
    set *s = setNew();
    int pathlen = traverseGraphRecursivePartOne(g, s, "start");
    setRelease(s);
    return pathlen;
}


int traverseGraphRecursivePartTwo(graph *g, dict *d, char *key, int seentwice) {
    list *l;
    graphNode *gn;
    int acc = 0, *visits;

    l = graphGet(g, key);
    visits = (int *)dictGet(d, key);
    (*visits)++;

    // This is a bit fragile
    if (*visits == 2 && islower(key[0])) seentwice = 1;
    if (strncmp(key, "end", 3) == 0) return 1;

    for (listNode *ln = l->head; ln != NULL; ln = ln->next) {
        gn = ln->value;

        if (strncmp(gn->name, "start", 5) == 0) continue;

        visits = (int *)dictGet(d, gn->name);

        /**
         * - big cave
         * - not visited
         * - small and not seen twice
         */
        if (!gn->islower || !*visits || (gn->islower && !seentwice)) {
            acc += traverseGraphRecursivePartTwo(g, d, gn->name, seentwice);
            (*visits)--; // ensure we can have another look
            if (*visits == 1 && gn->islower)
                seentwice = 0;
        }

    }
    return acc;
}

int problemTwoSolve(graph *g) {
    dict *d = dictNew();
    d->freeValue = xfree;
    dictEntry *de;
    int acc;

    // Setup visited count
    for (int i = 0; i < DICTCAPACITY; ++i) {
        if ((de = g->d->entries[i]) != NULL) {
            while (de) {
                if (de->key) {
                    int *tmp = xmalloc(sizeof(int));
                    *tmp = 0;
                    dictPut(d, de->key, tmp);
                }
                de = de->next;
            }
        }
    }
    
    acc = traverseGraphRecursivePartTwo(g, d, "start", 0);

    graphRelease(g);
    dictRelease(d);
    return acc;
}

int main(int argc, char **argv) {
    rFile *rf = rFileRead("./simple.txt");
    graph *g = problemToGraph(rf->buf);

    if (argc == 1) {
        fprintf(stderr, "Usage: %s <c1|c2>\n"
                "c1: challenge 1 and c2: challenge2\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (strncmp(argv[1], "c2", 2) == 0) {
        printf("challenge2: %d\n", problemOneSolve(g));
    } else if (strncmp(argv[1], "c1", 2) == 0) {
        printf("challenge1: %d\n", problemTwoSolve(g));
    } else {
        fprintf(stderr, "Invalid option: \"%s\" valid options are c1 or c2",
                argv[1]);
        exit(EXIT_FAILURE);
    }

    graphRelease(g);
    rFileRelease(rf);
}
