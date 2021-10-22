#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define NUMBER_OF_PRINTS 10

sem_t *sem1;
sem_t *sem2;
pid_t parent;
pid_t child;

// man sem_destroy :
//       The effect of destroying a semaphore upon which
//       other threads  are currently blocked is undefined.
void stop_parent() {
    if (-1 != parent) {
        kill(parent, SIGINT);
        printf("I...I killed my father...\n");
    }
}

void destroy_semaphores() {
    sem_close(sem1);
    sem_close(sem2);
}

void fatal_exit() {
    destroy_semaphores();
    stop_parent();
    exit(EXIT_FAILURE);
}

int initialize() {
    if (SEM_FAILED == (sem1 = sem_open("/sem1", O_CREAT, 0644, 1))) {
        return EXIT_FAILURE;
    }
    if (SEM_FAILED == (sem2 = sem_open("/sem2", O_CREAT, 0644, 0))) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int initialize_child() {
    if (SEM_FAILED == (sem1 = sem_open("/sem1", 0))) {
        return EXIT_FAILURE;
    }
    if (SEM_FAILED == (sem2 = sem_open("/sem2", 0))) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void *second_printer(void *_) {

    if (EXIT_FAILURE == initialize_child()) {
        fatal_exit();
    }

    for (int i = 0; i < NUMBER_OF_PRINTS; ++i) {
        if (sem_wait(sem2) != 0) {
            fatal_exit();
        }
        printf("Second-%d\n", i);
        if (sem_post(sem1) != 0) {
            fatal_exit();
        }
    }

    return (void *) EXIT_SUCCESS;
}

void *first_printer(void *_) {
    for (int i = 0; i < NUMBER_OF_PRINTS; ++i) {
        if (0 != sem_wait(sem1)) {
            fatal_exit();
        }
        printf("First-%d\n", i);
        if (0 != sem_post(sem2)) {
            fatal_exit();
        }
    }

    return (void *) EXIT_SUCCESS;
}


int main() {
    if (EXIT_SUCCESS != initialize()) {
        return EXIT_FAILURE;
    }

    parent = getpid();

    if (0 != (child = fork())) {
        parent = -1;
        first_printer(NULL);
    } else {
        second_printer(NULL);
    }

    if (child == 0) {
        destroy_semaphores();
        printf("Child finished\n");
    } else {
        printf("Parent finished\n");
    }
    return EXIT_SUCCESS;
}