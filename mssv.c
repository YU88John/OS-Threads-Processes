#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define SIZE 9

int Sol[SIZE][SIZE];            /* Sudoku solution grid */
int Row[SIZE] = {0};            /* Validation status of each row */
int Col[SIZE] = {0};            /* Validation status of each column */
int Sub[SIZE] = {0};            /* Validation status of each sub-grid */
int Counter = 0;                /* Total count of valid rows, columns, and sub-grids */
int completed_threads = 0;      /* Number of threads that have completed their tasks */
int last_thread_id = 0;         /* ID of the last thread to complete */
pthread_mutex_t lock;           /* Mutex for thread synchronization */
pthread_cond_t cond;            /* Condition variable for thread signaling */

/* Function to read the Sudoku solution from a file */
void readSolution(const char* filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Unable to open file");
        exit(EXIT_FAILURE);
    }

    /* Reading the Sudoku solution grid from file */
    int i, j;
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            if (fscanf(file, "%d", &Sol[i][j]) != 1) {
                fprintf(stderr, "Error reading Sudoku grid from file\nThe file is invalid. It must be a 9x9 grid.\n");
                fclose(file);
                exit(EXIT_FAILURE);
            }
        }
    }

    fclose(file);

    /* Check if the grid size is exactly 9x9 
    if (i != SIZE || j != SIZE) {
        fprintf(stderr, "The file is invalid. It must be a 9x9 grid.\n");
        exit(EXIT_FAILURE);
    } */
}

/* Function to validate a sub-grid */
int validateSubgrid(int startRow, int startCol) {
    int check[SIZE + 1] = {0};   /* Array to check presence of numbers 1-9 */
    int i, j;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            int num = Sol[startRow + i][startCol + j];
            if (check[num] == 1) {
                return 0;       /* Invalid sub-grid if number appears more than once */
            }
            check[num] = 1;     /* Mark number as seen */
        }
    }
    return 1;                   /* Sub-grid is valid */
}

/* Function to validate a column */
int validateColumn(int col) {
    int check[SIZE + 1] = {0};   /* Array to check presence of numbers 1-9 */
    int i;
    for (i = 0; i < SIZE; i++) {
        int num = Sol[i][col];
        if (check[num] == 1) {
            return 0;           /* Invalid column if number appears more than once */
        }
        check[num] = 1;         /* Mark number as seen */
    }
    return 1;                   /* Column is valid */
}

/* Function to validate a row */
int validateRow(int row) {
    int check[SIZE + 1] = {0};   /* Array to check presence of numbers 1-9 */
    int i;
    for (i = 0; i < SIZE; i++) {
        int num = Sol[row][i];
        if (check[num] == 1) {
            return 0;           /* Invalid row if number appears more than once */
        }
        check[num] = 1;         /* Mark number as seen */
    }
    return 1;                   /* Row is valid */
}

/* Thread function for validating sub-grids and columns */
void* validateSubgridsAndColumns(void* param) {
    int threadId = *(int*)param;
    int startSubgrid = (threadId - 1) * 3;
    int startColumn = (threadId - 1) * 3;

    printf("Thread ID-%d is validating sub-grids and columns\n", threadId);

    int valid = 1; /* Flag to check if the thread's work is valid */
    int i;

    /* Validate 3 sub-grids */
    for (i = 0; i < 3; i++) {
        sleep(1);   /* Simulate work */
        int validSub = validateSubgrid((startSubgrid + i) / 3 * 3, (startSubgrid + i) % 3 * 3);
        pthread_mutex_lock(&lock);   /* Lock mutex before accessing shared resources */
        Sub[startSubgrid + i] = validSub;    /* Update sub-grid validation status */
        if (validSub) {
            Counter++;    /* Increment counter if sub-grid is valid */
        } else {
            printf("Thread ID-%d: Sub-grid %d is invalid\n", threadId, startSubgrid + i + 1);
            valid = 0;
        }
        pthread_mutex_unlock(&lock); /* Unlock mutex after accessing shared resources */
    }

    /* Validate 3 columns */
    for (i = 0; i < 3; i++) {
        sleep(1);   /* Simulate work */
        int validCol = validateColumn(startColumn + i);
        pthread_mutex_lock(&lock);   /* Lock mutex before accessing shared resources */
        Col[startColumn + i] = validCol;    /* Update column validation status */
        if (validCol) {
            Counter++;    /* Increment counter if column is valid */
        } else {
            printf("Thread ID-%d: Column %d is invalid\n", threadId, startColumn + i + 1);
            valid = 0;
        }
        pthread_mutex_unlock(&lock); /* Unlock mutex after accessing shared resources */
    }

    /* Update completion status and last thread ID */
    pthread_mutex_lock(&lock);   /* Lock mutex before accessing shared resources */
    completed_threads++;         /* Increment number of completed threads */
    last_thread_id = threadId;   /* Record current thread ID as last completed */
    pthread_cond_signal(&cond);  /* Signal waiting threads that a thread has completed */
    pthread_mutex_unlock(&lock); /* Unlock mutex after accessing shared resources */

    if (valid) {
        printf("Thread ID-%d is valid\n", threadId);
    }

    pthread_exit(NULL);          /* replace return(NULL)*/
}

/* Thread function for validating rows */
void* validateRows(void* param) {
    int threadId = *(int*)param;
    printf("Thread ID-%d is validating rows\n", threadId);

    int valid = 1; /* Flag to check if the thread's work is valid */
    int i;

    /* Validate each row */
    for (i = 0; i < SIZE; i++) {
        sleep(1);   /* Simulate work */
        int validRow = validateRow(i);
        pthread_mutex_lock(&lock);   /* Lock mutex before accessing shared resources */
        Row[i] = validRow;      /* Update row validation status */
        if (validRow) {
            Counter++;    /* Increment counter if row is valid */
        } else {
            printf("Thread ID-%d: Row %d is invalid\n", threadId, i + 1);
            valid = 0;
        }
        pthread_mutex_unlock(&lock); /* Unlock mutex after accessing shared resources */
    }

    /* Update completion status and last thread ID */
    pthread_mutex_lock(&lock);   /* Lock mutex before accessing shared resources */
    completed_threads++;         /* Increment number of completed threads */
    last_thread_id = threadId;  /* Record thread ID as last completed */
    pthread_cond_signal(&cond);  /* Signal waiting threads that a thread has completed */
    pthread_mutex_unlock(&lock); /* Unlock mutex after accessing shared resources */

    if (valid) {
        printf("Thread ID-%d is valid\n", threadId);
    }

    pthread_exit(NULL);          /* Terminate the thread */
}

int main(int argc, char* argv[]) {
    /* Check command-line arguments */
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <solution_file> <delay>\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* Check delay value */
    int delay = atoi(argv[2]);
    if (delay < 1 || delay > 10) {
        fprintf(stderr, "The delay should be between 1 to 9 seconds\n");
        return EXIT_FAILURE;
    }

    /* Read Sudoku solution from file */
    readSolution(argv[1]);

    /* Initialize mutex and condition variable */
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);

    pthread_t threads[4];   /* Array to hold thread IDs */
    int threadIds[4] = {1, 2, 3, 4};    /* Thread IDs for validation threads */

    /* Create threads for validating sub-grids/columns and rows */
    int i;
    for (i = 0; i < 4; i++) {
        pthread_create(&threads[i], NULL, (i < 3) ? validateSubgridsAndColumns : validateRows, &threadIds[i]);
    }

    /* Wait for all threads to complete with pthread_join(). wait for each thread */
    for (i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL); 
    }

    /* Output validation results for rows, columns, and sub-grids */
    printf("Validation Results:\n");
    for (i = 0; i < SIZE; i++) {
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

    /* Print the valid threads */
    printf("Valid threads:\n");
    for (i = 0; i < 4; i++) {
        if (Row[i] && Col[i] && Sub[i]) {
            printf("Thread ID-%d is valid\n", i + 1);
        }
    }

    /* Cleanup mutex and condition variable */
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);

    return EXIT_SUCCESS;
}
