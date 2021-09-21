#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define MUTEX_COUNT 3
#define NUMBER_OF_PRINTS 10

pthread_mutexattr_t mutex_attribute;
pthread_mutex_t mutexes[MUTEX_COUNT];

void destroyMutexes(int mutex_count) {
    for (int i = 0; i < mutex_count; ++i) {
        if (pthread_mutex_destroy(&mutexes[i]) != 0) {
            perror("Error destoying mutex");
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
    if (pthread_mutex_lock(&mutexes[mutex_index])) {
        fatalExit("Error locking mutex");
    }
}

void unlockMutex(int mutex_index) {
    if (pthread_mutex_unlock(&mutexes[mutex_index])) {
        fatalExit("Error unlocking mutex");
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

void *secondPrint(void *_) {
    lockMutex(2);
    for (int i = 0; i < NUMBER_OF_PRINTS; ++i) {
        lockMutex(1);
        printf("Second: %d\n", i);
        unlockMutex(2);
        lockMutex(0);
        unlockMutex(1);
        lockMutex(2);
        unlockMutex(0);
    }
    unlockMutex(2);
    return NULL;
}

void firstPrint() {
    //Locks first in main
    for (int i = 0; i < NUMBER_OF_PRINTS; ++i) {
        printf("First: %d\n", i);
        lockMutex(0);
        unlockMutex(1);
        lockMutex(2);
        unlockMutex(0);
        lockMutex(1);
        unlockMutex(2);
    }
    unlockMutex(1);
}

//Second
void *print2(void *_) {
    for (int i = 0; i < NUMBER_OF_PRINTS; ++i) {
        lockMutex(0);
        printf("ABOBICH\n");
        lockMutex(1);
    }
}

//First
void *print(void *_) {
    //Locked 0
    for (int i = 0; i < NUMBER_OF_PRINTS; ++i) {
        printf("ABOBA\n");
        unlockMutex(0);
        lockMutex(1);
    }
}

int main() {
    pthread_t child;

    initMutexes();
    lockMutex(1);

    if (pthread_create(&child, NULL, secondPrint, NULL)) {
        fatalExit("Error creating thread");
    }

    firstPrint();

    if (pthread_join(child, NULL)) {
        fatalExit("Error waiting thread");
    }

    destroyMutexes(MUTEX_COUNT);
    return EXIT_SUCCESS;
}