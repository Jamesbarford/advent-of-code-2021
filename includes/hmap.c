 /**
 * Version 1.0 October 2021 - simple hashtable
 *
 * Copyright (c) 2021, James Barford-Evans
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hmap.h"

#define _hmapShouldRebuild(h)                                                  \
    ((h)->size >= (h)->rebuildThreashold && (h)->fixedsize == 0)

static inline hmapEntry **hmapEntryAlloc(unsigned int capacity) {
    return calloc(capacity, sizeof(hmapEntry *));
}

static inline unsigned int hashString(void *key) {
    char *s = key;
    unsigned int h = (unsigned int)*s;

    if (h) {
        for (++s; *s; ++s)
            h = (h << 5) - h + (unsigned int)*s;
    }

    return h;
}

static inline unsigned int hashNumber(void *s) {
    unsigned int *x = s;
    unsigned int tmp = *x;

    tmp = ((tmp >> 16) ^ tmp) * 0x45d9f3b;
    tmp = ((tmp >> 16) ^ tmp) * 0x45d9f3b;
    tmp = (tmp >> 16) ^ tmp;
    return tmp;
}

static inline int hmapNumCmp(void *k1, unsigned int h1, void *k2,
        unsigned int h2)
{
    double *n1 = k1;
    double *n2 = k2;

    return h1 == h2 && *n1 == *n2;
}

static inline int hmapStrCmp(void *k1, unsigned int h1, void *k2,
        unsigned int h2)
{
    char *str1 = k1;
    char *str2 = k2;

    return h1 == h2 && strcmp(str1, str2) == 0;
}

static unsigned int roundup32bit(unsigned int num) {
    num--;
    num |= num >> 1;
    num |= num >> 2;
    num |= num >> 4;
    num |= num >> 8;
    num |= num >> 16;
    num++;
    return num;
}

static void _hmapEntryRelease(hmap *hm, hmapEntry *he, int freeHe) {
    if (he) {
        if (freeHe) {
            hmapKeyRelease(hm, he->key);
            hmapValueRelease(hm, he->value);
            free(he);
        }
    }
}

hmapType defaultType = {
    .hashFn = hashString,
    .keycmp = hmapStrCmp,
    .freekey = free,
    .freevalue = free,
};

void hmapRelease(hmap *hm) {
    hmapEntry *he, *next, *cur;

    for (unsigned int i = 0; i < hm->size; ++i) {
        if ((he = hm->entries[i]) != NULL) {
            if (he->next != NULL) {
                cur = he->next;

                while (cur) {
                    next = cur->next;
                    _hmapEntryRelease(hm, cur, 1);
                    cur = next;
                }
            }
            _hmapEntryRelease(hm, he, 1);
        }
    }

    hm->size = 0;
    hm->mask = 0;
    hm->rebuildThreashold = 0;
    free(hm->entries);
    free(hm);
}

static hmap *_hmapCreate(hmapType *type, int fixedsize,
        unsigned int startCapacity)
{
    hmap *hm;

    if ((hm = malloc(sizeof(hmap))) == NULL)
        return NULL;

    if ((hm->entries = hmapEntryAlloc(HM_MIN_CAPACITY)) == NULL) {
        free(hm);
        return NULL;
    }

    hm->type = type == NULL ? &defaultType : type;
    hm->capacity = startCapacity;
    hm->size = 0;
    hm->mask = hm->capacity - 1;
    hm->rebuildThreashold = ~~((unsigned int)(HM_LOAD * hm->capacity));
    hm->fixedsize = fixedsize;

    return hm;
}

int khashcmp(char *s1, unsigned int h1, char *s2, unsigned int h2) {
    return h1 == h2 && (strcmp(s1, s2) == 0);
}

hmap *hmapCreateWithType(hmapType *type) {
    return _hmapCreate(type, 0, HM_MIN_CAPACITY);
}

hmap *hmapCreate() {
    return hmapCreateWithType(&defaultType);
}

hmap *hmapCreateFixed(hmapType *type, unsigned int capacity) {
    return _hmapCreate(type, 1, roundup32bit(capacity));
}

static int _hmapExpand(hmap *hm) {
    unsigned int i;
    int idx;
    hmapEntry **newEntries, **oldEntries;
    unsigned int newCapacity, newMask, newRebuildThreashold, size;

    size = hm->size;

    newCapacity = hm->capacity << 1;
    newMask = newCapacity - 1;
    newRebuildThreashold = ~~((unsigned int)(HM_LOAD * newCapacity));

    newEntries = hmapEntryAlloc(newCapacity);
    if (newEntries == NULL)
        return HM_ERR;

    for (i = 0; i < hm->capacity && size > 0; ++i) {
        hmapEntry *he, *nextHe;

        if (hm->entries[i] == NULL)
            continue;

        he = hm->entries[i];
        while (he) {
            idx = he->hash & newMask;
            nextHe = he->next;
            he->next = newEntries[idx];
            newEntries[idx] = he;
            size--;
            he = nextHe;
        }
    }

    oldEntries = hm->entries;
    hm->entries = newEntries;
    hm->mask = newMask;
    hm->capacity = newCapacity;
    hm->rebuildThreashold = newRebuildThreashold;
    free(oldEntries);
    return HM_OK;
}

static int _hmapGetIndex(hmap *hm, void *key, unsigned int hash) {
    hmapEntry *he;
    int idx;

    if (_hmapShouldRebuild(hm))
        _hmapExpand(hm);

    idx = hash & hm->mask;
    he = hm->entries[idx];

    while (he) {
        if (hmapKeycmp(hm, key, hash, he->key, he->hash))
            return HM_FOUND;
        he = he->next;
    }

    return idx;
}

hmapEntry *hmapGetEntry(hmap *hm, void *key) {
    hmapEntry *he;
    unsigned int idx, hash;

    hash = hmapHash(hm, key);
    idx = hash & hm->mask;
    he = hm->entries[idx];

    while (he) {
        if (hmapKeycmp(hm, key, hash, he->key, he->hash))
            return he;
        he = he->next;
    }

    return NULL;
}

int hmapContains(hmap *hm, void *key) {
    unsigned int hash = hmapHash(hm, key);
    if (_hmapGetIndex(hm, key, hash) == HM_FOUND)
        return HM_FOUND;
    return HM_NOT_FOUND;
}

int hmapAdd(hmap *hm, void *key, void *value) {
    int idx;
    unsigned int hash;
    hmapEntry *newHe;

    hash = hmapHash(hm, key);

    if ((idx = _hmapGetIndex(hm, key, hash)) == HM_FOUND)
        return HM_FOUND;

    if ((newHe = malloc(sizeof(hmapEntry))) == NULL)
        return HM_ERR;

    newHe->next = hm->entries[idx];
    newHe->key = key;
    newHe->value = value;
    newHe->hash = hash;
    hm->entries[idx] = newHe;
    hm->size++;

    return HM_OK;
}

/**
 * Return HM_OK on success else HM_NOT_FOUND if it does not
 * exist
 */
int hmapDelete(hmap *hm, void *key) {
    unsigned int hash, idx;
    hmapEntry *he, *heprev;

    heprev = NULL;
    hash = hmapHash(hm, key);
    idx = hash & hm->mask;
    he = hm->entries[idx];

    while (he) {
        if (hmapKeycmp(hm, key, hash, he->key, he->hash)) {
            if (heprev) {
                heprev->next = he->next;
            } else {
                hm->entries[idx] = he->next;
            }
            hm->size--;
            _hmapEntryRelease(hm, he, 1);
            return HM_OK;
        } else {
            heprev = he;
            he = he->next;
        }
    }

    return HM_NOT_FOUND;
}

hmapIterator *hmapIteratorCreate(hmap *hm) {
    hmapIterator *iter;

    if ((iter = malloc(sizeof(hmapIterator))) == NULL)
        return NULL;

    iter->idx = 0;
    iter->hm = hm;
    iter->cur = NULL;

    return iter;
}

void hmapIteratorRelease(hmapIterator *iter) {
    if (iter) free(iter);
}

int hmapIteratorGetNext(hmapIterator *iter) {
    unsigned int idx;

    while (iter->idx < iter->hm->capacity) {
        if (iter->cur == NULL || iter->cur->next == NULL) {
            idx = iter->idx;
            iter->idx++;

            if (iter->hm->entries[idx] != NULL) {
                iter->cur = iter->hm->entries[idx];
                return 1;
            }
        } else if (iter->cur->next != NULL) {
            iter->cur = iter->cur->next;
            return 1;
        } else {
            return 0;
        }
    }

    return 0;
}
