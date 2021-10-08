#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>

#define ITERATIONS_WITHOUT_CHECK 10000000

pthread_barrier_t barrier;
pthread_mutex_t mutex;
int is_alive = 1;

typedef struct Data {
    int start_position;
    int threads_amount;
    double result;
} Data;

void *signal_handler_thread(void *_) {
    sigset_t sig_set;
    int ret_sig;

    sigaddset(&sig_set, SIGINT);

    if (0 != pthread_sigmask(SIG_BLOCK, &sig_set, NULL)) {
        printf("Can't init sigmask in thread\n");
        exit(EXIT_FAILURE);
    }

    sigwait(&sig_set, &ret_sig);
    if (SIGINT == ret_sig) {
        pthread_mutex_lock(&mutex);
        is_alive = 0;
        pthread_mutex_unlock(&mutex);
    }
}

void *calculate(void *data) {
    int iteration = 0;
    double pi = 0.0;
    Data *local_data = (Data *) data;
    int start = local_data->start_position;
    int thread_amount = ((Data *) data)->threads_amount;

    for (;; start += thread_amount) {
        pi += 1.0 / (start * 4.0 + 1.0);
        pi -= 1.0 / (start * 4.0 + 3.0);
        iteration++;
        if (ITERATIONS_WITHOUT_CHECK == iteration) {
            pthread_barrier_wait(&barrier);
            pthread_mutex_lock(&mutex);
            if (!is_alive) {
                pthread_mutex_unlock(&mutex);
                local_data->result = pi;
                pthread_exit(EXIT_SUCCESS);
            }
            pthread_mutex_unlock(&mutex);
            iteration = 0;
        }
    }
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
        } else {
            printf("Joined thread\n");
        }
        result += database[i].result;
    }
    result *= 4;
    printf("Result value: %lf\n", result);
    destroyAll(database, threads);

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    if (pthread_mutex_init(&mutex, NULL)) {
        printf("Can't init mutex\n");
        return EXIT_FAILURE;
    }

    sigset_t blocking_set;
    sigaddset(&blocking_set, SIGINT);
    if (0 != pthread_sigmask(SIG_BLOCK, &blocking_set, NULL)) {
        printf("Can't init sigmask\n");
        return EXIT_FAILURE;
    }

    if (argc == 2) {
        int threads_amount = atoi(argv[1]);
        if (pthread_barrier_init(&barrier, NULL, threads_amount)) {
            return EXIT_FAILURE;
        }
        pthread_t thread_signal_handler;
        if (pthread_create(&thread_signal_handler, NULL, signal_handler_thread, NULL) != 0) {
            printf("Can't init thread_signal_handler\n");
            return EXIT_FAILURE;
        }
        countPi(threads_amount);
        if (pthread_barrier_destroy(&barrier) != 0) {
            printf("DESTROY error\n");
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    } else {
        printf("Pass count of threads as argument\n");
        return EXIT_FAILURE;
    }
}