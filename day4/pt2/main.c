#include <stdio.h>
#include <stdlib.h>

#include "../../includes/readfile.h"
#include "../../includes/xmalloc.h"

#define isnumber(x) ((x) >= 48 && (x) <= 57)
#define DEL    1
#define ACTIVE 0
#define RUN_COMPLETE 1
#define REMOVED 101 // this is dreadful but the bingo bingo numbers only go upto 99

/**
 * To make life a bit simpler, we're going to use some simple
 * data structures
 *
 * ==== STRUCTURES BEGIN ====
 */
typedef struct vector {
    unsigned int len;
    unsigned int capacity;
    int *entries;
} vector;

#define vectorGet(v, i) ((v)->entries[(i)])
#define vectorSet(v, i, x) ((v)->entries[(i)] = (x), (v)->len++)

vector *vectorNew(unsigned int capacity) {
    vector *v;
    v = xmalloc(sizeof(vector));
    v->entries = xcalloc(capacity, sizeof(int));
    v->len = 0;
    v->capacity = capacity;
    return v;
}

void vectorRelease(vector *v) {
    if (v) {
        xfree(v->entries);
        xfree(v);
    }
}

void vectorPrint(vector *v) {
    printf("length: %u\n", v->len);
    printf("capacity: %u\n", v->capacity);
    printf("[");
    for (unsigned int i = 0; i < v->len; ++i) {
        if (i + 1 != v->len)
            printf("%d, ", vectorGet(v, i));
        else
            printf("%d", vectorGet(v, i));
    }
    printf("]\n");
}

typedef struct matrix {
    unsigned int rows;
    unsigned int columns;
    char *entries;
} matrix;

matrix *matrixNew(unsigned int rows, unsigned int columns) {
    matrix *m;
    m = xmalloc(sizeof(matrix));
    m->entries = xcalloc(columns * rows, sizeof(char));
    m->rows = rows;
    m->columns = columns;
    return m;
}

void matrixRelease(matrix *m) {
    if (m) {
        xfree(m->entries);
        xfree(m);
    }
}

#define matrixGet(m, x, y) ((m)->entries[(x) + (y) * (m)->columns])
#define matrixSet(m, x, y, n) ((m)->entries[(x) + (y) * (m)->columns] = (n))

void matrixPrint(matrix *m) {
    printf("rows: %u\n", m->rows);
    printf("columns: %u\n", m->columns);
    printf("[\n");
    int val;

    for (unsigned int y = 0; y < m->rows; ++y) {
        printf("  [");
        for (unsigned x = 0; x < m->columns; ++x) {
            val = matrixGet(m, x, y);
            if (x + 1 != m->columns)
                printf("%2d, ", val);
            else
                printf("%2d", val);
        }
        if (y + 1 != m->rows)
            printf("],\n");
        else
            printf("]\n");
    }
    printf("]\n");
}

unsigned int matrixSum(matrix *bm) {
    unsigned int acc, x, y;
    int val;

    for (y = 0, acc = 0; y < bm->rows; ++y) {
        for (x = 0; x < bm->columns; ++x) {
            val = matrixGet(bm, x, y);
            if (val == REMOVED) continue;
            acc += val;
        }
    }

    return acc;
}

/**
 * ==== STRUCTURES END ==== */



static vector *problemGetBingoNumbers(char *buf, int *first_line_len) {
    int i, linelen, num;
    vector *v;
    char *ptr;

    ptr = buf;
    linelen = 0;
    
    for (ptr = buf; *ptr != '\n'; ptr++, linelen++);

    // this will over allocate but thats okay :)
    v = vectorNew(linelen);

    /* Fill up vector with ints */
    for (ptr = buf, i = 0; *ptr != '\n'; ptr++, i++) {
        num = 0;
        while (*ptr != ',' && *ptr != '\n')
            num = num * 10 + (*(ptr++) - 48);
        vectorSet(v, i, num);
    }

    *first_line_len = linelen;

    return v;
}

/**
 * - Add spaces for column count
 * - Add '\n' for row count
 * - terminate on \n\n
 *
 * We do this so we can know how big the matrix's will need to be
 */
void problemGetRowsAndColumns(char *buf, int offset, unsigned int *rows,
        unsigned int *columns, int *board_charlen)
{
    char *ptr;
    int new_line_count;

    *rows = 0;
    *columns = 0;
    *board_charlen = 0;

    /* count ' ' to get number of columns */
    for (ptr = buf + offset; *ptr != '\n'; ptr++) {
        if (*ptr == ' ') (*columns)++;
        (*board_charlen)++;
    }

    /* we've gone through the first row */
    (*rows)++;
    /* advance to next row */
    ptr++;

    /* and count \n to get number of rows */
    new_line_count = 0;
    while (new_line_count != 2) {
        if (*(ptr++) == '\n') {
            new_line_count++;
            if (new_line_count == 1)
                (*rows)++;
        } else new_line_count = 0;
    }
} 

/**
 * Fill `n` number of matrix's from the file.
 * - single digits have 2 pieces of whitespace before them
 */
matrix **problemGetBingoBoards(char *buf, int offset, unsigned int rows,
        unsigned int columns, int board_count)
{
    matrix **bm;
    char *ptr;
    int i, num;
    int x, y;

    /* setup boards */
    bm = xmalloc(sizeof(matrix) * board_count);
    for (i = 0; i < board_count; ++i)
        bm[i] = matrixNew(rows, columns);

    i = 0;
    ptr = buf + offset;

    x = 0;
    y = 0;
    while (*ptr != '\0') {
        /* single digits have 2 pieces of whitespace */
        while (!isnumber(*ptr))
            ptr++;
        while (*ptr != '\n') {
            num = 0;
            while (*ptr != ' ' && *ptr != '\n')
                num = num * 10 + (*(ptr++) - 48);
            matrixSet(bm[i], x, y, num);
            if (*ptr == '\n') break;
            while (!isnumber(*ptr))
                ptr++;
            x++;
        }
        ptr++;
        y++;
        x = 0;
        if (*ptr == '\n') {
            i++;
            /* advance past \n\n */
            ptr++;
            y = 0;
        }
    }

    return bm;
}

unsigned int problemGetBoardCount(char *buf, int offset) {
    unsigned int boardcount;
    boardcount = 0;

    for (char *ptr = buf + offset; *ptr != '\0'; ptr++) {
        if (*ptr == '\n' && *(ptr + 1) == '\n') boardcount++; 
    }

    boardcount++;

    return boardcount;
}

/* Is there a column with all zeros? */
int matixCheckColumns(matrix *m) {
    unsigned int x, y;
    int run = 0;

    for (x = 0; x < m->columns; ++x) {
        run = 0;
        for (y = 0; y < m->rows; ++y) {
            if (matrixGet(m, x, y) == REMOVED)
                run++;
        }
        if (run == 5) return RUN_COMPLETE;
    }

    return run == 5;
}

/* Is there a row all zeros? */
int matrixCheckRows(matrix *m) {
    unsigned int x, y;
    int run = 0;

    for (y = 0; y < m->rows; ++y) {
        run = 0;
        for (x = 0; x < m->columns; ++x)
            if (matrixGet(m, x, y) == REMOVED)
                run++;
        if (run == 5) return RUN_COMPLETE;
    }

    return run == 5;
}

/* If there is a match on a number mark it as 'REMOVED' */
void matrixUnsetNumberOnMatch(matrix *m, int num) {
    for (unsigned int y = 0; y < m->rows; ++y)
        for (unsigned int x = 0; x < m->columns; ++x)
            if (num == matrixGet(m, x, y))
                matrixSet(m, x, y, REMOVED);
}

int main(void) {
    rFile *rf;
    vector *bingo_numbers;
    matrix **boards, *board;
    char *deleted_boards;
    unsigned int rows, columns, i, active_boards, total;
    int first_line_len, board_count, board_charlen, curnum, curboard;

    rf =  rFileRead("../input.txt");

    bingo_numbers = problemGetBingoNumbers(rf->buf, &first_line_len);
    problemGetRowsAndColumns(rf->buf, first_line_len + 2, &rows, &columns, &board_charlen);

    /* there is probably a smater way of doing this */
    board_count = problemGetBoardCount(rf->buf, first_line_len+2);
    boards = problemGetBingoBoards(rf->buf, first_line_len + 2, rows, columns,
                        board_count);

    curboard = 0;
    curnum = 0;

    /* keep track of what boards are active and what are dead */
    deleted_boards = xcalloc(board_count, sizeof(char));
    active_boards = board_count;

    for (i = 0; i < bingo_numbers->len; ++i) {
        curnum = bingo_numbers->entries[i];
        for (curboard = 0; curboard < board_count; ++curboard)
            matrixUnsetNumberOnMatch(boards[curboard], curnum);

        /**
         * - If board is marked as deleted, skip
         * - See if Rows are all marked
         * OR
         * - See if Columns are all marked
         * - if the are flag the board as removed and decrement the number of active boards
         *
         * Mark a deleted board in 'deleted_boards'
         */
        for (curboard = 0; curboard < board_count; ++curboard) {
            board = boards[curboard];
            if (deleted_boards[curboard] == 1) continue;

            if (matrixCheckRows(board) == RUN_COMPLETE) {
                deleted_boards[curboard] = 1;
                active_boards--;
            } else if (matixCheckColumns(board) == RUN_COMPLETE) {
                deleted_boards[curboard] = 1;
                active_boards--;
            }

            /* We've removed the final board so can exit these loops */
            if (active_boards == 0) goto answer;
        }
    }

answer:
    total = matrixSum(boards[curboard]);
    printf("%u\n", total * curnum);

    vectorRelease(bingo_numbers);
    for (i = 0; i < (unsigned int)board_count; ++i)
        matrixRelease(boards[i]);
    xfree(boards);
    rFileRelease(rf);
}
