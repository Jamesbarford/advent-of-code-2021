#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "../includes/readfile.h"
#include "../includes/xmalloc.h"

#define DICTCAPACITY (1 << 10)
#define DSTRINGSIZ (1 << 16)
#define ALLOC_SIZE (1L<<32L)

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
    long value;
    unsigned int hash;
    struct dictEntry *next;
} dictEntry;

dictEntry *dictEntryNew(char *key, long value, unsigned int hash) {
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

long dictGet(dict *d, char *key) {
    dictEntry *de = dictFind(d, key);
    return de ? de->value : 0;
}

int dictHas(dict *d, char *key) {
    return dictFind(d, key) != NULL;
}

void dictPut(dict *d, char *key, long value) {
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
                printf("[%s] => %ld\n", de->key, de->value);
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

dict *problemToDict(char *buf, char *firstline) {
    dict *d = dictNew();
    int offset = 0;
    char *ptr, *keyptr, value;
    
    ptr = buf;
    while (*ptr != '\n')
        firstline[offset++] = (*ptr++);

    firstline[offset] = '\0';
    offset += 2;
    ptr = &buf[offset];

    while (1) {
        switch (buf[offset]) {
            case '\0': return d;
            case ' ':
                buf[offset] = '\0';
                keyptr = &buf[offset - 2];
                offset += 4;
                value = buf[offset];
                dictPut(d, keyptr, value);
                break;
        }
        offset++;
    }

    return d;
}

void getMinMax(char *solution, unsigned long *min, unsigned long *max) {
    char *ptr;
    unsigned long array[1<<16] = {0};
    unsigned long i = 0;
    for (ptr = solution; *ptr != '\0'; ptr++) {
        array[(int)*ptr-65]++;
        i++;
    }

    *max = 0;
    *min = 10000000;
    for (unsigned long j = 0; j < 1<<16; ++j) {
        if (array[j] > *max) *max = array[j];
        else if (array[j] > 0 && array[j] < *min) *min = array[j];
    }
}

/**
 * This blows my computers brains out if used for part2
 */
unsigned long solveProblemOne(dict *rules, char *puzzle, int iters) {
    unsigned long i = 0;
    char *solution = xcalloc(ALLOC_SIZE, sizeof(char));
    char key[3] = {'\0'};
    char *ptr, value;

    ptr = puzzle;
    unsigned long max = 0, min = 10000000;

    for (int s = 0; s < iters; ++s) {
        while(*ptr != '\0' && *(ptr + 1) != '\0') { 
            key[0] = *ptr;
            key[1] = *(ptr + 1);
            key[2] = '\0';

            value = dictGet(rules, key);
            solution[i] = *ptr;
            solution[i+1] = value;
            solution[i+2] = *(ptr + 1);
            solution[i+3] = '\0';

            i+=2;
            ptr++;
        }
        memcpy(puzzle, solution, sizeof(char) * i + 1);
        ptr = puzzle;
        i = 0;
    }

    getMinMax(solution, &min, &max);

    xfree(solution);
    return max-min;
}

void dictClear(dict *d) {
    dictReleaseEntries(d);
    d->entries = xcalloc(DICTCAPACITY, sizeof(dictEntry));
}

/**
 * This is leaking tonnes of memory with the strdup, but must press on!
 */
unsigned long solveProblemTwo(dict *rules, char *puzzle, int iters) {
    long count = 0;
    char key[3] = {'\0'}, *tmpkey;
    char *ptr, rule;
    dict *s_dict = dictNew();
    dict *s_dict2 = dictNew();
    unsigned long charcount[97] = {0};
    unsigned long max = 0, min = LONG_MAX;

    ptr = puzzle;

    while(*ptr != '\0') { 
        key[0] = *ptr;
        key[1] = *(ptr + 1);
        key[2] = '\0';

        count = dictGet(s_dict, key);
        charcount[(int)*ptr] += 1;
        dictHas(s_dict, key) ? (tmpkey=key) : (tmpkey=strdup(key));
        dictPut(s_dict, tmpkey, count + 1);
        ptr++;
    }

    for (int i = 0; i < iters; ++i) {
        for (int k = 0; k < DICTCAPACITY; ++k) {
            dictEntry *de;
            if ((de = s_dict->entries[k]) != NULL) {
                if (de->next != NULL)
                    printf("not null\n");
                if (dictHas(rules, de->key)) {
                    rule = dictGet(rules, de->key);
                    count = dictGet(s_dict, de->key);

                    charcount[(int)rule] += count;

                    key[0] = de->key[0]; 
                    key[1] = rule;
                    key[2] = '\0';
                    long value = dictGet(s_dict2, key);
                    dictPut(s_dict2, strdup(key), count+value);

                    key[0] = rule; 
                    key[1] = de->key[1];
                    key[2] = '\0';
                    value = dictGet(s_dict2, key);
                    dictPut(s_dict2, strdup(key), count+value);

                }
            }
        }
        dictEntry **tmp = s_dict->entries;
        s_dict->entries = s_dict2->entries;
        s_dict2->entries = tmp;
        dictClear(s_dict2);
    }

    dictRelease(s_dict2);
    dictRelease(s_dict);

    for (int i = 0; i < 97; ++i) {
        if (charcount[i] > 0 && charcount[i] < min) min = charcount[i];
        if (charcount[i] > max) max = charcount[i];
    }

    printf("min: %ld, max:%ld\n", min, max);

    return max-min;
}

int main(int argc, char **argv) {
    rFile *rf = rFileRead("./input.txt");
    char *puzzle = xcalloc(ALLOC_SIZE, sizeof(char));
    dict *rules = problemToDict(rf->buf, puzzle);

    if (argc == 1) {
        fprintf(stderr, "Usage: %s <c1|c2>\n"
                "c1: challenge 1 and c2: challenge2\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (strncmp(argv[1], "c1", 2) == 0) {
        printf("challenge1: %ld\n", solveProblemOne(rules, puzzle, 10));
    } else if (strncmp(argv[1], "c2", 2) == 0) {
        printf("challenge2: %ld\n", solveProblemTwo(rules, puzzle, 40));
    } else {
        fprintf(stderr, "Invalid option: \"%s\" valid options are c1 or c2",
                argv[1]);
        exit(EXIT_FAILURE);
    }

    dictRelease(rules);
}
