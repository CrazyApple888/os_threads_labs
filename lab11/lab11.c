#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define SUCCESS 0
#define FAIL 1
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
        return FAIL;
    }
    if (pthread_cond_destroy(&condition)) {
        return FAIL;
    }

    return SUCCESS;
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
        return FAIL;
    }
    if (pthread_cond_init(&condition, NULL)) {
        return FAIL;
    }

    return SUCCESS;
}

void *do_job(void *arg) {
    Data *data = (Data *) arg;
    int isFirst = data->isFirst;

    if (pthread_mutex_lock(&mutex)) {
        fatalExit("Mutex lock");
    }
    for (int i = 0; i < NUMBER_OF_PRINTS; ++i) {
        if (isFirst != turn) {
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
    if (initialize() != 0) {
        perror("Error in initialize");
        return EXIT_FAILURE;
    }

    Data first = {1, "I AM FIRST"};
    Data second = {0, "I AM SECOND"};

    if (pthread_create(&child, NULL, do_job, &second) == 0) {
        do_job(&first);
    } else {
        printf("Can't create thread\n");
        return 1;
    }
    if (destroy()) {
        perror("Destroy");
        return 1;
    }
    return 0;
}
