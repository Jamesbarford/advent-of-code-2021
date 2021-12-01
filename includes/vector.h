/**
 * Version 1.0 October 2021 - vector implementation
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
#ifndef __VECTOR_H__
#define __VECTOR_H__

#define VEC_ERR 0
#define VEC_OK  1

typedef struct vectorEntry {
    char empty;
    void *data;
} vectorEntry;

typedef struct vector {
    unsigned int size;
    unsigned int capacity;
    unsigned int resizeThreashold;
    vectorEntry *entries;
    void (*freeItem)(void *);
    int (*match)(void *, void *);
    void (*print)(void *);
} vector;

typedef struct vectorStats {
    double min;
    double max;
    double range;
    double avg;
} vectorStats;

/**
 * Unsafe exists for when we already know `i` is within the bounds of a vector
 * and saves us a NULL check when iterating over a massive array.
 *
 * MUST use bounds checking before this function. It exists so within a loop
 * that we know already is within the bounds of a vector there is no checking
 * for NULL on each iteration
 */
#define vectorGetUnsafe(v, i) (&(v)->entries[(i)]);

/* ensure to cast value */
#define vectorGetValueUnsafe(v, i) ((v)->entries[(i)].data)

/* Get a value from a vector, allows for 
 * generic opperations */
typedef double vectorAccessor(void *);

static inline double vectorAccessorGet(vector *v, unsigned int idx,
        vectorAccessor *accessor)
{
    return accessor(v->entries[idx].data);
}

/* Usecase where the data stored is a double */
static inline double vectorDoubleAccessor(void *data) {
    return *(double *)data;
}

vector *vectorCreate();
void vectorRelease(void *v);

/**
 * This is for the scenario where the ->data points to an address
 * that has been allocated as one E.G:
 *
 * vector *v = vectorCreate();
 * double *arr = (double *)malloc(sizeof(double) * 2);
 *
 * arr[0] = 99.1;
 * arr[1] = 132.45;
 *
 * v->entries[0].data = &arr; // This is where we can call free;
 * v->entries[1].data = &(arr + 1); // This will seg fault.
 *
 * This can emerge if you've wrapped a conventional array as a vector.
 */
void vectorReleaseSingleAlloc(void *v);

/* Setters & getters */

vectorEntry *vectorGet(vector *v, unsigned int idx);
void *vectorGetValue(vector *v, unsigned int idx);
void *vectorTail(vector *v);
void *vectorHead(vector *v);
int vectorSet(vector *v, unsigned int idx, void *value);
int vectorPush(vector *v, void *item);
void *vectorShift(vector *v);
void *vectorPop(vector *v);
void *vectorFind(vector *v, unsigned int startIdx,
        unsigned int endIdx, void *needle, unsigned int *foundIdx);
void vectorPrint(vector *v);

/* Maths */

double vectorChangeBetween(vector *v, unsigned int startIdx,
        unsigned int endIdx, unsigned int interval, vectorAccessor *accessor);

double vectorPercentChange(vector *v, unsigned int startIdx,
        unsigned int endIdx, vectorAccessor *accessor);

double vectorVariance(vector *v, unsigned int startIdx, unsigned int endIdx,
        double average, vectorAccessor *accessor);

int vectorGetStats(vector *v, vectorStats *vs, unsigned int startIdx,
        unsigned int endIdx, vectorAccessor *accessor);
#endif
