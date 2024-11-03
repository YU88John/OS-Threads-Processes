#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define SIZE 9

int Sol[SIZE][SIZE];
int Row[SIZE] = {0};
int Col[SIZE] = {0};
int Sub[SIZE] = {0};
int Counter = 0;
int last_thread_id = 0;
pthread_mutex_t lock;
pthread_cond_t cond;

// Function to read the Sudoku solution from a file
void readSolution(const char* filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Unable to open file");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fscanf(file, "%d", &Sol[i][j]);
        }
    }

    fclose(file);
}

// Function to validate a sub-grid
int validateSubgrid(int startRow, int startCol) {
    int check[SIZE + 1] = {0};
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            int num = Sol[startRow + i][startCol + j];
            if (check[num] == 1) {
                return 0;
            }
            check[num] = 1;
        }
    }
    return 1;
}

// Function to validate a column
int validateColumn(int col) {
    int check[SIZE + 1] = {0};
    for (int i = 0; i < SIZE; i++) {
        int num = Sol[i][col];
        if (check[num] == 1) {
            return 0;
        }
        check[num] = 1;
    }
    return 1;
}

// Function to validate a row
int validateRow(int row) {
    int check[SIZE + 1] = {0};
    for (int i = 0; i < SIZE; i++) {
        int num = Sol[row][i];
        if (check[num] == 1) {
            return 0;
        }
        check[num] = 1;
    }
    return 1;
}

// Thread function for validating sub-grids and columns
void* validateSubgridsAndColumns(void* param) {
    int threadId = *(int*)param;
    int startSubgrid = (threadId - 1) * 3;
    int startColumn = (threadId - 1) * 3;

    for (int i = 0; i < 3; i++) {
        sleep(1);
        int validSub = validateSubgrid((startSubgrid + i) / 3 * 3, (startSubgrid + i) % 3 * 3);
        pthread_mutex_lock(&lock);
        Sub[startSubgrid + i] = validSub;
        if (validSub) Counter++;
        pthread_mutex_unlock(&lock);
    }

    for (int i = 0; i < 3; i++) {
        sleep(1);
        int validCol = validateColumn(startColumn + i);
        pthread_mutex_lock(&lock);
        Col[startColumn + i] = validCol;
        if (validCol) Counter++;
        pthread_mutex_unlock(&lock);
    }

    pthread_mutex_lock(&lock);
    last_thread_id = threadId;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&lock);

    return NULL;
}

// Thread function for validating rows
void* validateRows(void* param) {
    for (int i = 0; i < SIZE; i++) {
        sleep(1);
        int validRow = validateRow(i);
        pthread_mutex_lock(&lock);
        Row[i] = validRow;
        if (validRow) Counter++;
        pthread_mutex_unlock(&lock);
    }

    pthread_mutex_lock(&lock);
    last_thread_id = *(int*)param;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&lock);

    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <solution_file> <delay>\n", argv[0]);
        return EXIT_FAILURE;
    }

    readSolution(argv[1]);
    int delay = atoi(argv[2]);

    pthread_t threads[4];
    int threadIds[4] = {1, 2, 3, 4};

    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);

    pthread_create(&threads[0], NULL, validateSubgridsAndColumns, &threadIds[0]);
    pthread_create(&threads[1], NULL, validateSubgridsAndColumns, &threadIds[1]);
    pthread_create(&threads[2], NULL, validateSubgridsAndColumns, &threadIds[2]);
    pthread_create(&threads[3], NULL, validateRows, &threadIds[3]);

    pthread_mutex_lock(&lock);
    for (int i = 0; i < 4; i++) {
        pthread_cond_wait(&cond, &lock);
    }
    pthread_mutex_unlock(&lock);

    for (int i = 0; i < SIZE; i++) {
        if (Row[i] == 0) {
            printf("Row %d is invalid\n", i + 1);
        }
        if (Col[i] == 0) {
            printf("Column %d is invalid\n", i + 1);
        }
        if (Sub[i] == 0) {
            printf("Sub-grid %d is invalid\n", i + 1);
        }
    }
    printf("Total valid rows, columns, and sub-grids: %d\n", Counter);

    printf("Thread ID-%d is the last thread.\n", last_thread_id);

    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);

    return 0;
}