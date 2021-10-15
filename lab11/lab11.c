#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define MUTEX_COUNT 3
#define NUMBER_OF_PRINTS 10

int is_ready = 1;

pthread_mutexattr_t mutex_attribute;
pthread_mutex_t mutexes[MUTEX_COUNT];

void destroyMutexes(int mutex_count) {
    for (int i = 0; i < mutex_count; ++i) {
        if (pthread_mutex_destroy(&mutexes[i]) != 0) {
            perror("Error destroying mutex");
            exit(EXIT_FAILURE);
        }
    }
}

void initMutexes() {
    pthread_mutexattr_init(&mutex_attribute);
    if (pthread_mutexattr_settype(&mutex_attribute, PTHREAD_MUTEX_ERRORCHECK)) {
        perror("Error creating attributes\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < MUTEX_COUNT; ++i) {
        if (pthread_mutex_init(&mutexes[i], &mutex_attribute)) {
            destroyMutexes(i);
            perror("Error creating mutex");
            exit(EXIT_FAILURE);
        }
    }
}

void fatalExit(char *error_message) {
    destroyMutexes(MUTEX_COUNT);
    perror(error_message);
    exit(EXIT_FAILURE);
}

void lockMutex(int mutex_index) {
    if (MUTEX_COUNT <= mutex_index || 0 != pthread_mutex_lock(&mutexes[mutex_index])) {
        fatalExit("Error locking mutex");
    }
}

void unlockMutex(int mutex_index) {
    if (MUTEX_COUNT <= mutex_index || 0 != pthread_mutex_unlock(&mutexes[mutex_index])) {
        fatalExit("Error unlocking mutex");
    }
}

void *firstPrinter(void *_) {
    //mutex 0 locked
    int i;
    for (i = 0; i < NUMBER_OF_PRINTS; ++i) {
        lockMutex((i + 1) % MUTEX_COUNT);
        printf("THREAD 1\n");
        unlockMutex(i % MUTEX_COUNT);
    }
    unlockMutex(i % MUTEX_COUNT);
    return (void*)EXIT_SUCCESS;
}

void *secondPrinter(void *_) {
    lockMutex(2);
    is_ready = 0;
    int i;
    for (i = 0; i < NUMBER_OF_PRINTS; ++i) {
        lockMutex(i % MUTEX_COUNT);
        printf("THREAD 2\n");
        unlockMutex((i + 2) % MUTEX_COUNT);
    }
    unlockMutex((i - 1) % MUTEX_COUNT);
    return (void*)EXIT_SUCCESS;
}

int main() {
    initMutexes();
    lockMutex(0);
    pthread_t child;
    if (pthread_create(&child, NULL, secondPrinter, NULL)) {
        fatalExit("Error creating thread");
    }
    while (is_ready != 0) {
        //Wait, until thread 2 locks mutex
    }
    firstPrinter(NULL);
    if (pthread_join(child, NULL)) {
        fatalExit("Error waiting thread");
    }
    destroyMutexes(MUTEX_COUNT);
    return EXIT_SUCCESS;
}