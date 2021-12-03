/**
 * Track number of times there was an increase given a
 * sliding window
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../includes/readfile.h"

#define TMP_SIZ 100
#define WINDOW_SIZE 3

int sumarray(int *arr, int len) {
    int acc = 0;
    for (int i = 0; i < len; ++i)
        acc += arr[i];
    return acc;
}

void printarr(int *arr, int len) {
    printf("[");
    for (int i = 0; i < len; ++i) {
        if (i + 1 != len)
            printf("%d, ", arr[i]);
        else
            printf("%d", arr[i]);
    }
    printf("]\n");
}

int main(void) {
    rFile *rf;
    char *ptr, *tmpptr, tmp[TMP_SIZ];
    int cur, prev, increases, count;
    int arr[WINDOW_SIZE];

    memset(arr, 0, sizeof(int) * WINDOW_SIZE);
    rf = rFileRead("./input.txt");
    ptr = rf->buf;
    tmpptr = tmp;
    prev = -1;
    cur = 0;
    increases = 0;
    count = 0;

    while (readline(&ptr, tmpptr, sizeof(tmp))) {
        if (count < WINDOW_SIZE) {
            arr[count++] = atoi(tmp);
            if (count == WINDOW_SIZE)
                prev = sumarray(arr, WINDOW_SIZE);
        } else {
            arr[2] = arr[1];
            arr[1] = arr[0];
            arr[0] = atoi(tmp);
            cur = sumarray(arr, WINDOW_SIZE);
            if (cur > prev) increases++;
        }

        tmpptr = tmp;
        prev = cur;
    }

    printf("increases: %d\n", increases);
    rFileRelease(rf);
}
