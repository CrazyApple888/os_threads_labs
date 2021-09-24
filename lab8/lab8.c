#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define STEPS 300000000
#define MAX_THREADS 16

typedef struct Data {
    int start_position;
    int threads_amount;
    double result;
} Data;

void *calculate(void *data) {
    double pi = 0.0;
    Data *local_data = (Data *) data;
    int start = local_data->start_position;
    int thread_amount = ((Data *) data)->threads_amount;
    for (; start < STEPS; start += thread_amount) {
        pi += 1.0 / (start * 4.0 + 1.0);
        pi -= 1.0 / (start * 4.0 + 3.0);
    }
    local_data->result = pi;

    pthread_exit(EXIT_SUCCESS);
}

void destroyAll(Data *database, pthread_t *threads) {
    if (NULL != database) {
        free(database);
    }
    if (NULL != threads) {
        free(threads);
    }
}

int countPi(int threads_amount) {
    pthread_t *threads = (pthread_t *) malloc(sizeof(pthread_t) * threads_amount);
    if (threads == NULL) {
        printf("Can't allocate memory\n");
        return EXIT_FAILURE;
    }
    Data *database = (Data *) malloc(sizeof(Data) * threads_amount);
    if (database == NULL) {
        printf("Can't allocate memory\n");
        free(threads);
        return EXIT_FAILURE;
    }
    for (int i = 0; i < threads_amount; ++i) {
        database[i].start_position = i;
        database[i].threads_amount = threads_amount;
        database[i].result = 0.0;
        if (pthread_create(&threads[i], NULL, calculate, &database[i]) != 0) {
            printf("Unable to create threads\n");
            destroyAll(database, threads);
            return EXIT_FAILURE;
        }
    }
    double result = 0.0;
    for (int i = 0; i < threads_amount; ++i) {
        if (pthread_join(threads[i], NULL) != 0) {
            printf("Unable to join thread #%d\n", i);
            return EXIT_FAILURE;
        }
        result += database[i].result;
    }
    result *= 4;
    printf("Result value: %lf\n", result);
    destroyAll(database, threads);

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    if (argc == 2) {
        int threads_amount = atoi(argv[1]);
        if (threads_amount <= 0) {
            printf("Incorrect argument. Requires number >= 1\n");
            return EXIT_FAILURE;
        }
        countPi(threads_amount);
        return EXIT_SUCCESS;
    }
    struct timespec start, end;

    printf("Starting tests...\n");
    for (int i = 1; i <= MAX_THREADS; ++i) {
        clock_gettime(CLOCK_REALTIME, &start);
        countPi(i);
        clock_gettime(CLOCK_REALTIME, &end);
        double result_time = (double)end.tv_sec - start.tv_sec +  0.000000001*(double)(end.tv_nsec-start.tv_nsec);
        printf("Time taken %lf on %d threads\n", result_time, i);
    }
}