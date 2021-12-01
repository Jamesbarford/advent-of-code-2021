/**
 * Version 1.0 October 2021 - vector implemention
 * Copyright (c) 2021, James Barford-Evans
 * All rights reserved.
 *
 * Constraints:
 * - Vector should not have holes in the array, things can get messy.
 * - Elements should ONLY be accessed using the getters and setters.
 * - Accessor MUST return a double if you want to use arithmetic operations.
 * - ALL operations MUST return a vector, no double *array. Unless we
 *   are trying to get a singular value then it must be a double.
 * - It never shrinks so hypothetically you could have hundreds of bytes
 *   of memory allocated but only one entry.
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vector.h"

#define _vectorShouldResize(v) ((v)->size * 4 >= (v)->resizeThreashold)

static inline unsigned int roundup32bit(unsigned int num) {
    num--;
    num |= num >> 1;
    num |= num >> 2;
    num |= num >> 4;
    num |= num >> 8;
    num |= num >> 16;
    num++;
    return num;
}

/* Assign `size` number of vectorEntry's*/
static vectorEntry *_vectorAlloc(unsigned int size) {
    vectorEntry *entries;

    if ((entries = calloc(roundup32bit(size), sizeof(vectorEntry))) == NULL)
        return NULL;

    return entries;
}

/* Create a vector `capacity` in size */
static vector *_vectorCreate(unsigned int capacity) {
    vector *v;

    if ((v = malloc(sizeof(vector))) == NULL)
        return NULL;
    
    v->capacity = capacity;
    v->size = 0;
    v->resizeThreashold = v->capacity * 3;
    v->freeItem = NULL;
    v->match = NULL;
    if ((v->entries = _vectorAlloc(v->capacity)) == NULL) {
        free(v);
        return NULL;
    }

    return v;
}

/* vector capacity is always a capacity to the power of 2*/
vector *vectorCreate() {
    return _vectorCreate(16);
}

/* Resize the vector to the next power of 2 freeing old entries*/
static int _vectorResize(vector *v) {
    unsigned int newCapacity;
    vectorEntry *newEntries;
    vectorEntry *oldEntries;

    newCapacity = v->capacity << 1;
    oldEntries = v->entries;

    if ((newEntries = malloc(newCapacity * sizeof(vectorEntry))) == NULL)
        return VEC_ERR;

    for (unsigned int i = 0; i < v->size; ++i) {
        newEntries[i] = oldEntries[i];
    }

    v->resizeThreashold = newCapacity * 3;
    v->entries = newEntries;
    free(oldEntries);
    v->capacity = newCapacity;

    return VEC_OK;
}

/* Completely free vector from memory along with any entries */
void vectorRelease(void *v) {
    vector *vec = v;
    if (vec) {
        vectorEntry *ve;
        /** 
         * Cannot free individual items of the array if there is no free
         * function pointer.
         */
        if (vec->freeItem) {
            for (unsigned int i = 0; i < vec->size; ++i) {
                ve = &(vec->entries[i]);
                if (ve->data != NULL) {
                    vec->freeItem(ve->data);
                }
            }
        }
        free(vec->entries);
        free(vec);
    }
}

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
void vectorReleaseSingleAlloc(void *v) {
    if (v) {
        vector *vec = v;
        if (vec->freeItem) {
            vec->freeItem(vec->entries[0].data);
        }
        free(vec->entries);
        free(vec);
    }
}

/**
 * Get vectorEntry at `idx` if the `idx` is within the bounds of `vector.size`.
 * Otherwise return NULL.
 */
vectorEntry *vectorGet(vector *v, unsigned int idx) {
    if (idx >= v->size) return NULL;
    return &v->entries[idx];
}

/**
 * Get value at `idx` or NULL if the value is not populated in the vector
 */
void *vectorGetValue(vector *v, unsigned int idx) {
    vectorEntry *ve = vectorGet(v, idx);
    return ve == NULL ? NULL : ve->data;
}

/* Is the idx within the bounds of the vector */
int vectorBoundsCheck(vector *v, unsigned int idx) {
    return idx <= v->size ? VEC_OK : VEC_ERR;
}

/* Get last value of the vector that has a value or NULL*/
void *vectorTail(vector *v) {
    return vectorGetValue(v, v->size-1);
}

/* Get last value of the vector that has a value or NULL*/
void *vectorHead(vector *v) {
    return vectorGetValue(v, 0);
}

/**
 * Set vectorEntry at `idx`'s value to `value` if
 * idx is within the bounds of the vector
 *
 * Will resize if needed.
 */
int vectorSet(vector *v, unsigned int idx, void *value) {
    if (idx >= v->size) return VEC_ERR;

    vectorEntry *ve = &(v->entries[idx]);

    if (ve->empty == 1) {
        if (_vectorShouldResize(v)) _vectorResize(v);
        v->size++;
    }

    ve->data = value;
    ve->empty = 0;
    return VEC_OK;
}

/**
 * Add to the end of the vector setting `vectorEntry`'s data to `value`
 */
int vectorPush(vector *v, void *value) {
    if (_vectorShouldResize(v)) _vectorResize(v);
    vectorEntry *ve = &(v->entries[v->size++]);
    ve->data = value;
    ve->empty = 0;
    return VEC_OK;
}

/**
 * Remove the first item in the vector this is not fast as it requires going
 * over the whole list moving everything by one.
 */
void *vectorShift(vector *v) {
    if (v->size == 0) return NULL;
    void *data;
    vectorEntry *ve;

    ve = &(v->entries[0]);
    data = ve->data;

    for (unsigned int i = 1; i < v->size; ++i) {
        v->entries[i-1] = v->entries[i];
    }
    v->size--;

    return data;
}

/**
 * Remove the last value from the vector or return NULL
 */
void *vectorPop(vector *v) {
    /* vector is empty */
    if (v->size == 0) return NULL;

    void *data;
    vectorEntry *ve;

    ve = &(v->entries[v->size-1]);
    data = ve->data;

    ve->empty = 1;
    ve->data = NULL;
    v->size--;

    return data;
}

/**
 * If match is NULL, return NULL
 * Else will perform an O(n) scan untill the element is found from startIdx
 * till endIdx
 *
 * If the return value of the function is NULL, the `foundIdx` will be zero.
 *
 * Is bounds checked
 */
void *vectorFind(vector *v, unsigned int startIdx, unsigned int endIdx,
        void *needle, unsigned int *foundIdx)
{
    *foundIdx = 0;
    if (v->match == NULL) return NULL;
    if (vectorBoundsCheck(v, startIdx) == VEC_ERR) return NULL;
    if (vectorBoundsCheck(v, endIdx) == VEC_ERR) return NULL;

    vectorEntry *ve;
    unsigned int i;

    for (i = startIdx; i < endIdx; ++i) {
        ve = vectorGetUnsafe(v, i);
        if (v->match(ve->data, needle)) {
            *foundIdx = i;
            return ve->data;
        }
    }

    return NULL;
}

/* print the vector as JSON */
void vectorPrint(vector *v) {
    printf("size: %d\ncapacity: %d\nresize threahold:%d\n",
            v->size, v->capacity, v->resizeThreashold);
    printf("[");
    for (unsigned int i = 0; i < v->size; ++i) {
        v->print(vectorGet(v, i)->data);
        if (i < v->size - 1) {
            printf(", ");
        }
    }
    printf("]\n");
}

/*============= ARITHMETIC ====================*/

/* percent change from start to end, neagtive means decrease */
static inline double percentChange(double end, double start) {
    return ((end-start)/start)*100;
}

/**
 * Get the change between the end idx (later entry in vector) from startIdx
 * (earlier entry in vector) over interval. This only makes sense if the vector
 * is holding data over time that is sorted
 *
 * is bounds checked
 */
double vectorChangeBetween(vector *v, unsigned int startIdx,
        unsigned int endIdx, unsigned int interval, vectorAccessor *accessor)
{
    if (vectorBoundsCheck(v, startIdx) == VEC_ERR) return VEC_ERR;
    if (vectorBoundsCheck(v, endIdx) == VEC_ERR) return VEC_ERR;

    return (vectorAccessorGet(v, endIdx, accessor) /
        vectorAccessorGet(v, startIdx, accessor)) / interval;
}

/**
 * Percentage change from startIdx to endIdx, is bounds checked.
 */
double vectorPercentChange(vector *v, unsigned int startIdx,
        unsigned int endIdx, vectorAccessor *accessor)
{
    if (vectorBoundsCheck(v, startIdx) == VEC_ERR) return VEC_ERR;
    if (vectorBoundsCheck(v, endIdx) == VEC_ERR) return VEC_ERR;

    return percentChange(vectorAccessorGet(v, endIdx, accessor),
            vectorAccessorGet(v, startIdx, accessor));
}

/**
 * Calculate the variance of the vector from startIdx to endIdx.
 *
 * is bounds checked
 */
double vectorVariance(vector *v, unsigned int startIdx, unsigned int endIdx,
        double average, vectorAccessor *accessor)
{
    if (vectorBoundsCheck(v, startIdx) == VEC_ERR) return VEC_ERR;
    if (vectorBoundsCheck(v, endIdx) == VEC_ERR) return VEC_ERR;

    double variance, x;
    unsigned int i;

    x = 0;
    variance = 0;

    for (i = startIdx; i < endIdx; ++i) {
        x = vectorAccessorGet(v, i, accessor) - average;
        variance += x * x;
    }

    return variance / (endIdx - startIdx);
}

double _getvariance(double *arr, double avg, unsigned int size) {
    double variance;
    variance = 0;

    for (unsigned int i = 0; i < size; ++i) {
        double x = arr[i] - avg;
        variance += x * x;
    }

    return variance / (size - 1);
}

/**
 * Get:
 * - min
 * - max
 * - average
 * - range
 *  stored in `vectorStats`
 *  
 *  Is bounds checked.
 */
int vectorGetStats(vector *v, vectorStats *vs, unsigned int startIdx,
        unsigned int endIdx, vectorAccessor *accessor)
{
    if (vectorBoundsCheck(v, startIdx) == VEC_ERR) return VEC_ERR;
    if (vectorBoundsCheck(v, endIdx) == VEC_ERR) return VEC_ERR;

    double acc;
    unsigned int count;

    acc = 0;
    count = 0;
    vs->min = 100000000;
    vs->max = -100000000;
    vs->range = 0;
    vs->avg = 0;

    for (unsigned int i = startIdx; i < endIdx; ++i) {
        double value = accessor(v->entries[i].data);

        if (value > vs->max) vs->max = value;
        if (value < vs->min) {
            
            vs->min = value;
        }
        acc += value;
        count++;
    }

    vs->avg = acc / count;
    vs->range = vs->max - vs->min;

    return VEC_OK;
}

double vectorGetAverage(vector *v, vectorAccessor *accessor) {
    double acc;
    acc = 0;

    for (unsigned int i = 0; i < v->size; ++i) {
        acc += accessor(vectorGetValue(v, i));
    }

    return acc / v->size;
}

#ifdef VEC_TEST
#include <time.h>
static double testNumbers[] = {17.1, 2.2, 9604, 10671, 462.42, 62456.13,
    1345.4, 7821.74, 662, 9791, 23442, 2341, 1.21, 2341, 14243,
    2341243, 42362, 6689.21};

static double testArray[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 18, 13, 14, 15, 16, 17, 18, 19, 20,
    21, 22, 100, 24, 24, 26, 27, 28, 29};

int approxEquals(double value, double other) {
    double epsilon = 1e-18;
    return fabs(value - other) < epsilon;
}

void TEST_vectorPrintDouble(void *data) {
    printf("%.6f", vectorDoubleAccessor(data));
}

vector *TEST_initTestVector(double *test, int len) {
    vector *v = vectorCreate();
    v->freeItem = NULL;

    for (int i = 0; i < len; ++i)
        vectorPush(v, &test[i]);

    return v;
}

int TEST_vectorMatch(void *_d1, void *_d2) {
    double d1, d2;
    d1 = *(double *)_d1;
    d2 = *(double *)_d2;

    return d1 == d2;
}

int TEST_vectorFind(vector *v) {
    unsigned int foundIdx;
    double *needle = &testNumbers[3];
    double *found = vectorFind(v, 0, v->size, needle, &foundIdx);

    int expected = *needle == *found && foundIdx == 3;
    
    printf("expected %.6f, found: %.6f, idx: %u, Pass: %s\n", *needle, *found,
            foundIdx, expected==1?"true":"false");

    return expected;
}
int TEST_vectorPercentChange(vector *v) {
    double change = vectorPercentChange(v, 2, 3, vectorDoubleAccessor);
    double expected = (double)(10671-9604)/9604*100;
    printf("Change: %.10f%%, %.10f\nequal: %s\n",
            (double)change, expected, change==expected ? "true":"false");
    return change == expected;
}

int TEST_vectorGetStats(vector *v) {
    vectorStats vs;
    vectorGetStats(v, &vs, 0, v->size, vectorDoubleAccessor);

    int allEqual = vs.min == 1.21 &&
        vs.max == 2341243 &&
        vs.avg == 140860.856111111119389534 &&
        vs.range == 2341241.79;
    printf("min: %.6f, max: %.6f, average: %.18f, range: %.6f\nequal: %s\n",
            vs.min, vs.max, vs.avg, vs.range, allEqual == 1 ? "true":"false");

    return allEqual;
}

int TEST_vectorVariance(vector *v) {
    vectorStats vs;
    vectorGetStats(v, &vs, 0, v->size, vectorDoubleAccessor);
    double variance = vectorVariance(v, 0, v->size, vs.avg, vectorDoubleAccessor);
    int isEqual = variance == 285063097082.52032470703125;

    printf("Variance: %.6f\nequal: %s\n", variance, isEqual ? "true" : "false");

    return isEqual;
}

void TEST_vectorPerf() {
    vector *v = vectorCreate();
    v->freeItem = free;
    int iters = 10000000;
    int *n;
    double elapsed;
    clock_t start, end;

	start = clock();
    for (int i = 0; i < iters; ++i) {
        vectorPush(v, &i);
    }
	end = clock();
	elapsed = (double)(end - start) / CLOCKS_PER_SEC * 1000;
	printf("pushing %d keys: %.03fms\n", iters, elapsed);

	start = clock();
    while ((n = vectorPop(v)) != NULL) {
        //free(n);
    }
	end = clock();
	elapsed = (double)(end - start) / CLOCKS_PER_SEC * 1000;
	printf("popping %d keys: %.03fms\n", iters, elapsed);

    vectorPrint(v);
    vectorRelease(v);
    printf("Released\n");
}

int main(void) {
    int len = 0;
    int failed = 0;
    vector *testVector = TEST_initTestVector(testNumbers,
            sizeof(testNumbers) / sizeof(double));
    testVector->print = TEST_vectorPrintDouble;
    testVector->match = TEST_vectorMatch;

    char *testBuf = malloc(sizeof(char) * 3000);

    printf("vector performance: \n");
    TEST_vectorPerf();
    printf("=======================\n");

    printf("vectorFind\n");
    if (TEST_vectorFind(testVector) != 1) {
        failed++;
        len += snprintf(testBuf, 3000, "vectorFind FAILED\n"); 
    }
    printf("=======================\n");
    
    printf("vectorPercentChange\n");
    if (TEST_vectorPercentChange(testVector) != 1) {
        failed++;
        len += snprintf(testBuf, 3000, "vectorPercentChange FAILED\n"); 
    }
    printf("=======================\n");
    
    printf("vectorGetStats\n");
    if (TEST_vectorGetStats(testVector) != 1) {
        failed++;
        len += snprintf(testBuf, 3000, "vectorGetStats FAILED\n"); 
    }
    printf("=======================\n");

    printf("vectorVariance\n");
    if (TEST_vectorVariance(testVector) != 1) {
        failed++;
        len += snprintf(testBuf, 3000, "vectorVariance FAILED\n"); 
    }
    printf("=======================\n");

    if (failed != 0) exit(EXIT_FAILURE);
    vectorRelease(testVector);
    exit(EXIT_SUCCESS);
}
#endif
