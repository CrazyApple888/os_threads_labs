#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define SLEEP_TIME 2

void *do_job(void *_) {
    while (1) {
        write(STDOUT_FILENO, "Hey\n", 4);
    }

    return NULL;
}

int main() {
    pthread_t child;
    if (pthread_create(&child, NULL, do_job, NULL) != 0) {
        printf("Can't create thread\n");
        return 1;
    }
    sleep(SLEEP_TIME);
    if (pthread_cancel(child) != 0) {
        printf("Can't cancel thread :(\n");
        return 1;
    } else {
        if (pthread_join(child, NULL) != 0) {
            printf("Can't join thread\n");
        }
        printf("Thread canceled :)\n");
    }
    return 0;
}