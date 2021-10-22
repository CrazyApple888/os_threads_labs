#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

#define NUMBER_OF_PRINTS 10

sem_t sem1;
sem_t sem2;

void destroy_semaphores() {
    sem_destroy(&sem1);
    sem_destroy(&sem2);
}

void fatal_exit() {
    destroy_semaphores();
    exit(EXIT_FAILURE);
}

int initialize() {
    if (sem_init(&sem1, 0, 1) || sem_init(&sem2, 0, 0)) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void *second_printer(void *_) {
    for (int i = 0; i < NUMBER_OF_PRINTS; ++i) {
        if (sem_wait(&sem2) != 0) {
            fatal_exit();
        }
        printf("THREAD 2\n");
        if (sem_post(&sem1) != 0) {
            fatal_exit();
        }
    }

    return (void*)EXIT_SUCCESS;
}

void *first_printer(void *_) {
    for (int i = 0; i < NUMBER_OF_PRINTS; ++i) {
        if (0 != sem_wait(&sem1)) {
            fatal_exit();
        }
        printf("THREAD 1\n");
        if (0 != sem_post(&sem2)) {
            fatal_exit();
        }
    }

    return (void*)EXIT_SUCCESS;
}


int main() {
    pthread_t child;

    if (EXIT_SUCCESS != initialize()) {
        return EXIT_FAILURE;
    }

    if (0 != pthread_create(&child, NULL, second_printer, NULL)) {
        fatal_exit();
    }

    first_printer(NULL);

    if (0 != pthread_join(child, NULL)) {
        fatal_exit();
    }

    destroy_semaphores();
    return EXIT_SUCCESS;
}