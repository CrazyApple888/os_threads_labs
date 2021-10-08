#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define NUMBER_OF_PRINTS 10

pthread_mutex_t mutex;
pthread_cond_t condition;
int turn = 1;

typedef struct Data {
    int isFirst;
    char *string;
} Data;

int destroy() {
    if (pthread_mutex_destroy(&mutex)) {
        return EXIT_FAILURE;
    }
    if (pthread_cond_destroy(&condition)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void fatalExit(char *error_message) {
    perror(error_message);
    if (destroy()) {
        perror("Error in destroy");
    }
    exit(EXIT_FAILURE);
}

int initialize() {
    if (pthread_mutex_init(&mutex, NULL)) {
        return EXIT_FAILURE;
    }
    if (pthread_cond_init(&condition, NULL)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void *do_job(void *arg) {
    Data *data = (Data *) arg;
    int isFirst = data->isFirst;

    if (pthread_mutex_lock(&mutex)) {
        fatalExit("Mutex lock");
    }
    for (int i = 0; i < NUMBER_OF_PRINTS; ++i) {
        while (isFirst != turn) {
            if (pthread_cond_wait(&condition, &mutex)) {
                fatalExit("Cond wait");
            }
        }
        printf("%s-%d\n", data->string, i);
        turn = !turn;
        if (pthread_cond_signal(&condition)) {
            fatalExit("Cond signal");
        }
    }
    if (pthread_mutex_unlock(&mutex)) {
        fatalExit("Mutex unlock");
    }
    pthread_exit(EXIT_SUCCESS);
}

int main() {
    pthread_t child;
    if (EXIT_SUCCESS != initialize()) {
        perror("Error in initialize");
        return EXIT_FAILURE;
    }

    Data first = {1, "I AM FIRST"};
    Data second = {0, "I AM SECOND"};

    if (pthread_create(&child, NULL, do_job, &second) == 0) {
        do_job(&first);
    } else {
        printf("Can't create thread\n");
        return EXIT_FAILURE;
    }
    if (destroy()) {
        perror("Destroy");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
