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
#ifndef __HMAP_H__
#define __HMAP_H__

#define HM_MIN_CAPACITY 1 << 16
#define HM_LOAD 0.67
#define HM_ERR 0
#define HM_OK 1
#define HM_FOUND -4
#define HM_NOT_FOUND 3

typedef int hmapKeyCompare(void *, unsigned int h1, void *, unsigned int h2);
typedef unsigned int hmapHashFunction(void *);

typedef struct hmapType {
    hmapKeyCompare *keycmp;
    hmapHashFunction *hashFn;
    void (*freekey)(void *);
    void (*freevalue)(void *);
} hmapType;

typedef struct hmapEntry {
    void *key;
    void *value;
    unsigned int hash;
    struct hmapEntry *next;
} hmapEntry;

typedef struct hmap {
    unsigned int size;
    unsigned int capacity;
    unsigned int mask;
    unsigned int rebuildThreashold;
    int fixedsize;
    hmapType *type;
    hmapEntry **entries;
} hmap;

typedef struct hmapIterator {
    unsigned int idx;
    hmap *hm;
    hmapEntry *cur;
} hmapIterator;

#define hmapHash(h, k) ((h->type)->hashFn((k)))

hmap *hmapCreate();
hmap *hmapCreateWithType(hmapType *type);
hmap *hmapCreateFixed(hmapType *type, unsigned int capacity);
void hmapRelease(hmap *hm);

int hmapContains(hmap *hm, void *key);
int hmapAdd(hmap *hm, void *key, void *value);
int hmapDelete(hmap *hm, void *key);
hmapEntry *hmapGetEntry(hmap *hm, void *key);

hmapIterator *hmapIteratorCreate(hmap *hm);
void hmapIteratorRelease(hmapIterator *iter);
int hmapIteratorGetNext(hmapIterator *iter);

static inline int hmapKeycmp(hmap *hm, void *k1, unsigned int h1, void *k2,
        unsigned int h2)
{
    return hm->type->keycmp(k1, h1, k2, h2);
}

static inline void hmapKeyRelease(hmap *hm, void *key) {
    hm->type->freekey(key);
}

static inline void hmapValueRelease(hmap *hm, void *key) {
    hm->type->freevalue(key);
}
#endif
