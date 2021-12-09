#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../../includes/xmalloc.h"
#include "../../includes/readfile.h"

#define DICTSIZE 10
#define CSTR_PAD 5
#define ERR -1
#define OK   1

typedef char cstr;

void cstrRelease(void *str) {
    if (str) {
        cstr *s = str;
        s -= CSTR_PAD;
        s[4] = 'p';
        xfree(s);
    }
} 

void cstrSetLen(cstr *str, unsigned int len) {
    unsigned char *_str = (unsigned char *)str;
    _str -= CSTR_PAD;
    _str[0] = (len >> 24);
    _str[1] = (len >> 16);
    _str[2] = (len >>  8);
    _str[3] = len;
    (_str)[4] = '\0';
}

/* Get the integer out of the string */
static inline unsigned int cstrlen(cstr *str) {
    unsigned char *_str = (unsigned char *)str;

    _str -= CSTR_PAD;

    return ((unsigned int)_str[0] << 24) |
           ((unsigned int)_str[1] << 16) |
           ((unsigned int)_str[2] << 8)  |
           _str[3];
}

int cstrToString(cstr *str, char *outbuf, unsigned int outbuflen) {
    unsigned int len = cstrlen(str);
    if (outbuflen < len) return ERR;

    memcpy(outbuf, str, len);
    outbuf[len] = '\0';

    return OK;
}

int cstrcmp(cstr *s1, cstr *s2) {
    return cstrlen(s1) == cstrlen(s2) && memcmp(s1, s2, cstrlen(s1)) == 0;
}

int cstrindex(cstr *s1, char c) {
    for (unsigned int i = 0; i < cstrlen(s1); ++i)
        if (s1[i] == c) return i;
    return ERR;
}

/* does s1 contain s2 */
int cstrContains(cstr *s1, cstr *s2) {
    unsigned int count = 0;
    for (unsigned int i = 0; i < cstrlen(s1); ++i) {
        for (unsigned int j = 0; j < cstrlen(s2); ++j) {
            if (s1[i] == s2[j]) {
                count++;
                if (count == cstrlen(s2)) goto out;
            }
        }
    }

out:
    return count == cstrlen(s2);
}

int cstrDifference(cstr *s1, cstr *s2) {
    unsigned int count = 0;
    unsigned int s1len = cstrlen(s1); 
    unsigned int s2len = cstrlen(s2);

    for (unsigned int i = 0; i < s1len; ++i) {
        for (unsigned int j = 0; j < s2len; ++j) {
            if (s1[i] == s2[j]) {
                count++;
            }
        }
    }

    return s2len - count;
}

cstr *cstrNew(char *original, unsigned int len) {
    cstr *outbuf;
    outbuf = xcalloc(sizeof(char), len + 1 + CSTR_PAD);
    outbuf += CSTR_PAD;
 
    cstrSetLen(outbuf, len);
    memcpy(outbuf, original, len);
    outbuf[len] = '\0';
    return outbuf;
}


void stringsort(char *str, int len) {
    int i, j;
    char tmp;

	for (i = 0; i < len - 1; i++) {
		for (j = i + 1; j < len; j++) {
            if (str[i] > str[j]) {
                tmp = str[i];
                str[i] = str[j];
                str[j] = tmp;
            }
        }
    }
}

/**
 *
 * if it is a 5 length string and not in 3 and not in 4 it is bottom left
 *
 *
 * 4,7,1 all share the righthand side with eachother
 *
 * a 2 shares top left with 1 & 7 & 4
 *
 */
typedef struct dict {
    cstr *entries[DICTSIZE];
} dict;

#define dictAdd(d, k, v) ((d)->entries[(k)] = (v))
#define dictGet(d, k) ((d)->entries[(k)])

dict *dictNew(void) {
    dict *d;
    d = xmalloc(sizeof(dict));
    return d;
}

int dictFind(dict *d, char *value) {
    cstr *needle = NULL;

    for (int i = 0; i < DICTSIZE; ++i) {
        if ((needle = dictGet(d, i)) != NULL) {
            if (strcmp(needle, value) == 0) {
                return i;
            }
        }
    }
    return ERR;
}

void dictEntriesRelease(dict *d) {
    for (int i = 0; i < DICTSIZE; ++i) {
        cstrRelease(d->entries[i]);
        d->entries[i] = NULL;
    }
}

void dictPrint(dict *d) {
    cstr *s;
    for (int i = 0; i < DICTSIZE; ++i) { 
        if ((s = dictGet(d, i)) != NULL) {
            printf("[%d]: %s\n",i, s);
        } else {
            printf("[%d]: (null) \n", i);
        }
    }
}

void dictRelease(dict *d) {
    for (int i = 0; i < DICTSIZE; ++i)
        cstrRelease(d->entries[i]);
    xfree(d);
}

struct listNode;
typedef struct listNode {
    cstr *value;
    struct listNode *next;
} listNode;

typedef struct list {
    listNode *head;
    int size;
} list;

listNode *listNodeNew(cstr *value) {
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

void listAdd(list *l, cstr *value) {
    listNode *ln;
    ln = listNodeNew(value);
    ln->next = l->head;
    l->head = ln;
    l->size++;
}

void listNodeRelease(listNode *ln) {
    xfree(ln);
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

void listPrint(list *l) {
    listNode *ln = l->head;
    int idx = 0;

    while (ln) {
        printf("[%d]: %s\n", idx, ln->value);
        idx++;
        ln = ln->next;
    }
}

int listRemoveFromIdx(list *l, int idx) {
    listNode *ln, *tmp, *prev;
    int i = 0;
    if (l->size == 0) return ERR;

    if (idx == 0) {
        tmp = l->head;
        l->head = NULL;
        l->head = tmp->next;
        xfree(tmp);
        l->size--;
        return OK;
    }

    ln = l->head;
    prev = NULL;
    while (ln) {
        if (i == idx) {
            tmp = ln;
            if (prev) {
                prev->next = ln->next;
                if (ln->next) {
                    prev->next->next = ln->next->next;
                }
            }
            xfree(ln);
            l->size--;
            return OK;
        }
        i++;
        prev = ln;
        ln = ln->next;
    }

    return ERR;
}

unsigned long listSum(list *l) {
    unsigned long acc = 0ul;
    listNode *ln = l->head;

    while (ln) {
        if (ln->value != NULL) {
            unsigned long sl = atoll(ln->value);
            printf("%ld\n", sl);
            acc +=sl;
        }
        ln = ln->next;
    }

    return acc;
}

int dictInsertThree(dict *d, list *l) {
    int idx = 0;
    listNode *ln = l->head;
    cstr *one = dictGet(d, 1);
    cstr *four = dictGet(d, 4);
    cstr *seven = dictGet(d, 7);

    while (ln) {
        if (ln->value == NULL) break;
        if (cstrlen(ln->value) == 5) {
            int contains1 = cstrContains(ln->value, one);
            int diff4 = cstrDifference(ln->value, four);
            int contains7 = cstrContains(ln->value, seven);

            if (contains1 && diff4 == 1 && contains7) {
                dictAdd(d, 3, ln->value);
                listRemoveFromIdx(l, idx);
                return idx;
            }
        }
        idx++;
        ln = ln->next;
    }

    return ERR;
}

int dictInsertFive(dict *d, list *l) {
    int idx = 0;
    listNode *ln = l->head;
    cstr *one = dictGet(d, 1);
    cstr *four = dictGet(d, 4);
    cstr *seven = dictGet(d, 7);
    cstr *three = dictGet(d, 3);

    while (ln) {
        if (!ln->value) break;
        if (cstrlen(ln->value) == 5) {
            int diff1 = cstrDifference(ln->value, one);
            int diff4 = cstrDifference(ln->value, four);
            int diff7 = cstrDifference(ln->value, seven);
            int diff3 = cstrDifference(ln->value, three);
                
            if (diff1 == 1 && diff4 == 1 && diff7 == 1 && diff3 == 1) {
                dictAdd(d, 5, ln->value);
                listRemoveFromIdx(l, idx);
                return idx;
            }
        }
        idx++;
        ln = ln->next;
    }

    return ERR;
}

int dictInsertTwo(dict *d, list *l) {
    int idx = 0;
    listNode *ln = l->head;

    while (ln) {
        if (cstrlen(ln->value) == 5) {
            dictAdd(d, 2, ln->value); 
            listRemoveFromIdx(l, idx);
            return OK;
        }
        idx++;
        ln = ln->next;
    }
    return ERR;
}

int dictInsertSix(dict *d, list *l) {
    int idx = 0;
    listNode *ln = l->head;
    cstr *eight = dictGet(d, 8);
    cstr *one = dictGet(d, 1);

    while (ln) {
        if (cstrlen(ln->value) == 6) {
            int diff1 = cstrDifference(ln->value, one);
            int diff8 = cstrDifference(ln->value, eight);

            if (diff1 == 1 && diff8 == 1) {
                dictAdd(d, 6, ln->value);
                listRemoveFromIdx(l, idx);
                return OK;
            }

        }
        idx++;
        ln = ln->next;
    }
    return ERR;
}

int dictInsertNine(dict *d, list *l) {
    int idx = 0;
    listNode *ln = l->head;
    cstr *five = dictGet(d, 5);

    while (ln) {
        if (cstrlen(ln->value) == 6) {
            int diff5 = cstrDifference(ln->value, five);

            if (diff5 == 0) {
                dictAdd(d, 9, ln->value);
                listRemoveFromIdx(l, idx);
                return OK;
            }
        }

        idx++;
        ln = ln->next;
    }

    return ERR;
}

int dictInsertZero(dict *d, list *l) {
    dictAdd(d, 0, l->head->value);
    listRemoveFromIdx(l, 0);
    return ERR;
}

unsigned long solveProblem(char *buf) {
    char tmp[100] = {'\0'};
    int tmpint[4];
    int intpos = 0;
    char *ptr, *tmpptr;
    unsigned long acc = 0;
    // will be reused
    dict *d = dictNew();
    list *l = listNew();

    tmpptr = tmp;
    ptr = buf;

    while (*ptr != '\0') {
        while (*ptr != '|') {
            *tmpptr++ = *(ptr++);
            if (*ptr == ' ') {
                *tmpptr = '\0';
                unsigned int slen = strlen(tmp);
                int stored = 0;
                stringsort(tmp, slen);
                cstr *strnum = cstrNew(tmp, slen);

                switch (slen) {
                    case 2:
                        dictAdd(d, 1, strnum);
                        stored = 1;
                        break;
                    case 4:
                        dictAdd(d, 4, strnum);
                        stored = 1;
                        break;
                    case 3:
                        dictAdd(d, 7, strnum);
                        stored = 1;
                        break;
                    case 7:
                        dictAdd(d, 8, strnum);
                        stored = 1;
                        break;
                }
                ptr++;
                if (stored != 1) {
                    cstr *value = cstrNew(tmp, slen);
                    listAdd(l, value);
                }
                tmpptr = tmp;
            }
        }
        ptr += 2;
        tmpptr = tmp;
        intpos = 0;

        (void)dictInsertThree(d, l);
        (void)dictInsertFive(d, l);
        (void)dictInsertTwo(d, l);
        (void)dictInsertSix(d, l);
        (void)dictInsertNine(d, l);
        (void)dictInsertZero(d, l);
        listRelease(l);

        while (*ptr != '\n') {
            *tmpptr++ = *(ptr++);
            if (*ptr == ' ' || *ptr == '\n' || *ptr == '\0') {
                *tmpptr = '\0';
                stringsort(tmp, strlen(tmp));
                int idx = dictFind(d, tmp);
                tmpint[intpos++] = idx;
                tmpptr = tmp;
                if (*ptr == '\n' || *ptr == '\0') {
                    break;
                }
                ptr++;
            }
        }

        int tmplen = snprintf(tmp, 5, "%d%d%d%d",
                tmpint[0], tmpint[1], tmpint[2], tmpint[3]);
        tmp[tmplen] = '\0';
        acc += atol(tmp);
        dictEntriesRelease(d);
        d = dictNew();
        l = listNew();
        ptr++;
    }
    return acc;
}

int main(void) {
    rFile *rf = rFileRead("../input.txt");
    unsigned long acc = solveProblem(rf->buf);

    printf("answer: %ld\n", acc);

    rFileRelease(rf);
}
