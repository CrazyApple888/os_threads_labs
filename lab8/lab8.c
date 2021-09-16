#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define STEPS 1000

typedef struct Data {
    int start_position;
    int threads_amount;
} Data;

void *calculate(void *data) {
    double* pi = (double*)malloc(sizeof(double));
    *pi = 0.0;
    int start = ((Data *) data)->start_position;
    int thread_amount = ((Data *) data)->threads_amount;
    for (; start < STEPS; start += thread_amount) {
        *pi += 1.0 / (start * 4.0 + 1.0);
        *pi -= 1.0 / (start * 4.0 + 3.0);
    }

    pthread_exit(pi);
}

void destroyAll(Data *database, pthread_t *threads) {
    if (NULL != database) {
        free(database);
    }
    if (NULL != threads) {
        free(threads);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Pass 1 argument, please\n");
        return EXIT_SUCCESS;
    }
    int threads_amount = atoi(argv[1]);
    if (threads_amount <= 0) {
        printf("Incorrect argument. Requires number >= 1\n");
    }
    pthread_t *threads = (pthread_t *) malloc(sizeof(pthread_t) * threads_amount);
    Data *database = (Data *) malloc(sizeof(Data) * threads_amount);
    for (int i = 0; i < threads_amount; ++i) {
        database[i].start_position = i;
        database[i].threads_amount = threads_amount;
        if (pthread_create(&threads[i], NULL, calculate, &database[i]) != 0) {
            printf("Unable to create threads\n");
            destroyAll(database, threads);
        }
    }
    double result = 0.0;
    double *pi;
    for (int i = 0; i < threads_amount; ++i) {
        if (pthread_join(threads[i], &pi) != 0) {
            printf("Unable to join thread #%d\n", i);
            continue;
        }
        if (NULL != pi) {
            result += *pi;
        }
    }
    result *= 4;
    printf("Result value: %lf\n", result);
    destroyAll(database, threads);
    free(pi);

    return EXIT_SUCCESS;
}