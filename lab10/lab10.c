#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define HUNGRY_PHILOSOPHERS 5
#define DELAY 3000
#define FOOD 5000

pthread_mutex_t forks[HUNGRY_PHILOSOPHERS];
pthread_t philosophers[HUNGRY_PHILOSOPHERS];
pthread_mutex_t food_mutex;

pthread_mutex_t cond_mutex;
pthread_cond_t eat_cond;
int eat = 0;

int food_on_table() {
    static int food = FOOD;
    int my_food;

    pthread_mutex_lock(&food_mutex);
    if (food > 0) {
        food--;
    }
    my_food = food;
    pthread_mutex_unlock(&food_mutex);
    return my_food;
}

void get_both_forks(int fork1, int fork2) {
    int isCaptured = 1;

    pthread_mutex_lock(&cond_mutex);
    while (0 != isCaptured) {
        isCaptured = pthread_mutex_trylock(&forks[fork1]);
        if (0 != isCaptured) {
            isCaptured = pthread_mutex_trylock(&forks[fork2]);
            if (0 != isCaptured) {
                pthread_mutex_unlock(&forks[fork1]);
            }
        }

        while (eat > 1) {
            pthread_cond_wait(&eat_cond, &cond_mutex);
        }
        eat += 1;
    }
    pthread_mutex_unlock(&cond_mutex);
}

void down_forks(int f1, int f2) {
    pthread_mutex_lock(&cond_mutex);
    pthread_mutex_unlock(&forks[f1]);
    pthread_mutex_unlock(&forks[f2]);
    eat -= 1;
    pthread_cond_broadcast(&eat_cond);
    pthread_mutex_unlock(&cond_mutex);
}

void *philosopher(void *num) {
    int id;
    int left_fork, right_fork, f;

    id = (int) num;
    printf("Philosopher %d sitting down to dinner.\n", id);
    right_fork = id;
    left_fork = id + 1;

    if (left_fork == HUNGRY_PHILOSOPHERS)
        left_fork = 0;

    while ((f = food_on_table())) {

        printf("Philosopher %d: get dish %d.\n", id, f);
        get_both_forks(left_fork, right_fork);

        printf("Philosopher %d: eating.\n", id);
        usleep(DELAY * (FOOD - f + 1));
        down_forks(right_fork, left_fork);
    }
    printf("Philosopher %d is done eating.\n", id);
    return EXIT_SUCCESS;
}

int main() {
    pthread_cond_init(&eat_cond, NULL);
    pthread_mutex_init(&cond_mutex, NULL);
    pthread_mutex_init(&food_mutex, NULL);
    for (int i = 0; i < HUNGRY_PHILOSOPHERS; i++) {
        pthread_mutex_init(&forks[i], NULL);
    }
    for (int i = 0; i < HUNGRY_PHILOSOPHERS; i++) {
        pthread_create(&philosophers[i], NULL, philosopher, (void *) i);
    }
    for (int i = 0; i < HUNGRY_PHILOSOPHERS; i++) {
        pthread_join(philosophers[i], NULL);
    }
    
    for (int i = 0; i < HUNGRY_PHILOSOPHERS; i++) {
        pthread_mutex_destroy(&forks[i]);
    }
    pthread_mutex_destroy(&cond_mutex);
    pthread_mutex_destroy(&food_mutex);
    pthread_cond_destroy(&eat_cond);

    return 0;
}