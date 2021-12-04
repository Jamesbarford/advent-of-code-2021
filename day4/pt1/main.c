#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "../../includes/readfile.h"
#include "../../includes/xmalloc.h"

#define isnumber(x) ((x) >= 48 && (x) <= 57)
#define DEL    1
#define ACTIVE 0

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
    /**
     * a row is an unsigned long, 64 bits
     *
     * This 'could' be:
     * - An array of int (boring) and 5*5*sizeof(int) = 200 bytes
     * - An array of char, as max val is 99, meaning 5*5*sizeof(char) = 25bytes
     *   Also quite boring, though most space efficient
     * - A long, 8 bytes, 8 * 5 = 40bytes. Not as space efficient, but more fun :)
     * */
    unsigned long *entries;
} matrix;

matrix *matrixNew(unsigned int rows, unsigned int columns) {
    matrix *m;
    m = xmalloc(sizeof(matrix));
    m->entries = xcalloc(columns, sizeof(unsigned long));
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

/**
 * x is assumed to be 0..n
 * and we need bytes hence multiplying by 8; 0*8 = 0 byte 1 etc...
 */
#define matrixGet(m, x, y)                                                     \
    ((m)->entries[(y)] >> ((unsigned long)(x)*8ul) & 0xFF)

#define matrixSet(m, x, y, n)                                                  \
    ((m)->entries[(y)] |= ((unsigned long)(n) << ((unsigned long)x * 8ul)))

#define matrixUnset(m, x, y)                                                   \
    ((m)->entries[(y)] &=                                                      \
        (~((1ul << ((8 * (x)) + 8)) - 1ul)) ^ ((1ul << ((x)*8)) - 1ul))

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

unsigned int matrixSum(matrix *bm) {
    unsigned int acc, x, y;

    for (y = 0, acc = 0; y < bm->rows; ++y)
        for (x = 0; x < bm->columns; ++x)
           acc += matrixGet(bm, x, y);

    return acc;
}

unsigned int problemGetBoardCount(char *buf, int offset) {
    unsigned int boardcount;
    boardcount = 0;

    for (char *ptr = buf+offset; *ptr != '\0'; ptr++) {
        if (*ptr == '\n' && *(ptr + 1) == '\n') boardcount++; 
    }

    boardcount++;

    return boardcount;
}

int main(void) {
    rFile *rf;
    vector *bingo_numbers;
    matrix **boards, *board;
    unsigned int rows, columns, x, y, i;
    unsigned long row, shift;
    int first_line_len, board_count, board_charlen, curnum, curboard, total;

    rf =  rFileRead("../input.txt");

    bingo_numbers = problemGetBingoNumbers(rf->buf, &first_line_len);
    problemGetRowsAndColumns(rf->buf, first_line_len + 2, &rows, &columns,
                            &board_charlen);
    /* there is probably a smater way of doing this */
    board_count = problemGetBoardCount(rf->buf, first_line_len+2);
    boards = problemGetBingoBoards(rf->buf, first_line_len + 2, rows, columns,
                        board_count);

    curboard = 0;
    curnum = 0;

    for (i = 0; i < bingo_numbers->len; ++i) {
        curnum = bingo_numbers->entries[i];
        for (curboard = 0; curboard < board_count; ++curboard) {
            board = boards[curboard];
            for (y = 0; y < board->rows; ++y) {
                for (x = 0; x < board->columns; ++x) {
                    if (curnum == (int)matrixGet(board, x, y)) {
                        matrixUnset(board, x, y);
                    }
                }
            }
        }

        for (curboard = 0; curboard < board_count; ++curboard) {
            board = boards[curboard];

            for (y = 0; y < board->rows; ++y) {
                row = board->entries[y];
                int run = 0;
                for (x = 0; x < board->columns; ++x) {
                    if ((int)(row >> ((unsigned long)x * 8ul) & 0xFF) == 0)
                        run++;
                }
                if (run == 5) goto out;
            }

            for (x = 0; x < board->columns; ++x) {
                shift = ((unsigned long)x * 8ul) & 0xFF;
                int run = 0;
                for (y = 0; y < board->rows; ++y) {
                    if ((int)((unsigned long)board->entries[y] >> shift) == 0)
                        run++;
                }
                if (run == 5) goto out;
            }
        }
    }

out:
    total = matrixSum(boards[curboard]);
    printf("%d\n", total * curnum);

    vectorRelease(bingo_numbers);
    for (y = 0; y < (unsigned int)board_count; ++y)
        matrixRelease(boards[y]);
    rFileRelease(rf);
}
