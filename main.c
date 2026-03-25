/*
 * CSC 345-02 - Project #2: Multithreaded Sudoku Validator
 *
 * Mode 1 (./main 1): 11 threads
 *   - 1 thread checks all 9 rows
 *   - 1 thread checks all 9 columns
 *   - 9 threads check each 3x3 subgrid
 *
 * Mode 2 (./main 2): 27 threads
 *   - 9 threads, each checks one row
 *   - 9 threads, each checks one column
 *   - 9 threads, each checks one 3x3 subgrid
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

/* Global 9x9 Sudoku board */
int board[9][9];

/*
 * results array:
 * Mode 1: indices 0=rows, 1=cols, 2-10=subgrids
 * Mode 2: indices 0-8=rows, 9-17=cols, 18-26=subgrids
 */
int results[27];

/* Structure for passing row/column start position to a thread */
typedef struct {
    int row;
    int column;
    int index; /* index into results array */
} parameters;

/* MODE 1 THREAD FUNCTIONS (11 threads) */

/* Thread that checks ALL 9 rows */
void *check_all_rows(void *param) {
    parameters *data = (parameters *)param;
    int valid = 1;

    for (int r = 0; r < 9 && valid; r++) {
        int seen[10] = {0};
        for (int c = 0; c < 9; c++) {
            int val = board[r][c];
            if (val < 1 || val > 9 || seen[val]) {
                valid = 0;
                break;
            }
            seen[val] = 1;
        }
    }

    results[data->index] = valid;
    free(data);
    return NULL;
}

/* Thread that checks ALL 9 columns */
void *check_all_cols(void *param) {
    parameters *data = (parameters *)param;
    int valid = 1;

    for (int c = 0; c < 9 && valid; c++) {
        int seen[10] = {0};
        for (int r = 0; r < 9; r++) {
            int val = board[r][c];
            if (val < 1 || val > 9 || seen[val]) {
                valid = 0;
                break;
            }
            seen[val] = 1;
        }
    }

    results[data->index] = valid;
    free(data);
    return NULL;
}

/* Thread that checks ONE 3x3 subgrid starting at (row, column) */
void *check_subgrid(void *param) {
    parameters *data = (parameters *)param;
    int seen[10] = {0};
    int valid = 1;

    for (int r = data->row; r < data->row + 3 && valid; r++) {
        for (int c = data->column; c < data->column + 3; c++) {
            int val = board[r][c];
            if (val < 1 || val > 9 || seen[val]) {
                valid = 0;
                break;
            }
            seen[val] = 1;
        }
    }

    results[data->index] = valid;
    free(data);
    return NULL;
}

/* MODE 2 THREAD FUNCTIONS (27 threads) */

/* Thread that checks ONE row */
void *check_one_row(void *param) {
    parameters *data = (parameters *)param;
    int seen[10] = {0};
    int valid = 1;

    for (int c = 0; c < 9; c++) {
        int val = board[data->row][c];
        if (val < 1 || val > 9 || seen[val]) {
            valid = 0;
            break;
        }
        seen[val] = 1;
    }

    results[data->index] = valid;
    free(data);
    return NULL;
}

/* Thread that checks ONE column */
void *check_one_col(void *param) {
    parameters *data = (parameters *)param;
    int seen[10] = {0};
    int valid = 1;

    for (int r = 0; r < 9; r++) {
        int val = board[r][data->column];
        if (val < 1 || val > 9 || seen[val]) {
            valid = 0;
            break;
        }
        seen[val] = 1;
    }

    results[data->index] = valid;
    free(data);
    return NULL;
}

/* HELPER: Read board from input.txt */
int read_board(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        return 0;
    }
    for (int r = 0; r < 9; r++) {
        for (int c = 0; c < 9; c++) {
            if (fscanf(fp, "%d", &board[r][c]) != 1) {
                fclose(fp);
                return 0;
            }
        }
    }
    fclose(fp);
    return 1;
}

/* HELPER: Print the board */
void print_board() {
    for (int r = 0; r < 9; r++) {
        for (int c = 0; c < 9; c++) {
            if (c > 0) printf(" ");
            printf("%d", board[r][c]);
        }
        printf("\n");
    }
}

/* MODE 1: Run with 11 threads */
void run_mode1() {
    /* 11 threads total: 1 row checker, 1 col checker, 9 subgrid checkers */
    pthread_t threads[11];
    int tid = 0;

    /* Clear results */
    memset(results, 0, sizeof(results));

    /* Thread 0: check all rows */
    parameters *p = (parameters *)malloc(sizeof(parameters));
    p->row = 0; p->column = 0; p->index = 0;
    pthread_create(&threads[tid++], NULL, check_all_rows, p);

    /* Thread 1: check all columns */
    p = (parameters *)malloc(sizeof(parameters));
    p->row = 0; p->column = 0; p->index = 1;
    pthread_create(&threads[tid++], NULL, check_all_cols, p);

    /* Threads 2-10: check each 3x3 subgrid */
    int idx = 2;
    for (int r = 0; r < 9; r += 3) {
        for (int c = 0; c < 9; c += 3) {
            p = (parameters *)malloc(sizeof(parameters));
            p->row = r; p->column = c; p->index = idx++;
            pthread_create(&threads[tid++], NULL, check_subgrid, p);
        }
    }

    /* Join all 11 threads */
    for (int i = 0; i < 11; i++) {
        pthread_join(threads[i], NULL);
    }
}

/* MODE 2: Run with 27 threads */
void run_mode2() {
    /* 27 threads: 9 row + 9 col + 9 subgrid */
    pthread_t threads[27];
    int tid = 0;

    /* Clear results */
    memset(results, 0, sizeof(results));

    /* Threads 0-8: one per row */
    for (int r = 0; r < 9; r++) {
        parameters *p = (parameters *)malloc(sizeof(parameters));
        p->row = r; p->column = 0; p->index = r;
        pthread_create(&threads[tid++], NULL, check_one_row, p);
    }

    /* Threads 9-17: one per column */
    for (int c = 0; c < 9; c++) {
        parameters *p = (parameters *)malloc(sizeof(parameters));
        p->row = 0; p->column = c; p->index = 9 + c;
        pthread_create(&threads[tid++], NULL, check_one_col, p);
    }

    /* Threads 18-26: one per 3x3 subgrid */
    int idx = 18;
    for (int r = 0; r < 9; r += 3) {
        for (int c = 0; c < 9; c += 3) {
            parameters *p = (parameters *)malloc(sizeof(parameters));
            p->row = r; p->column = c; p->index = idx++;
            pthread_create(&threads[tid++], NULL, check_subgrid, p);
        }
    }

    /* Join all 27 threads */
    for (int i = 0; i < 27; i++) {
        pthread_join(threads[i], NULL);
    }
}

/* MAIN */
int main(int argc, char *argv[]) {
    /* Validate arguments */
    if (argc != 2 || (argv[1][0] != '1' && argv[1][0] != '2')) {
        fprintf(stderr, "Usage: %s <1|2>\n", argv[0]);
        return 1;
    }

    int mode = atoi(argv[1]);

    /* Read board */
    if (!read_board("input.txt")) {
        fprintf(stderr, "Error: could not read input.txt\n");
        return 1;
    }

    /* Print board state */
    printf("BOARD STATE IN input.txt:\n");
    print_board();

    /* Start timer */
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    /* Run selected mode */
    if (mode == 1) {
        run_mode1();
    } else {
        run_mode2();
    }

    /* Stop timer */
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) +
                     (end.tv_nsec - start.tv_nsec) / 1e9;

    /* Check all results */
    int total = (mode == 1) ? 11 : 27;
    int valid = 1;
    for (int i = 0; i < total; i++) {
        if (!results[i]) {
            valid = 0;
            break;
        }
    }

    /* Print result */
    printf("SOLUTION: %s (%.4f seconds)\n", valid ? "YES" : "NO", elapsed);

    return 0;
}
