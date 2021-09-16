#include <stdio.h>
#include <pthread.h>

void* do_job(void *_) {
    for (int i = 0; i < 10; ++i) {
        printf("Hello from new thread!\n");
    }
    double * some_value = (double *)malloc(sizeof(double));
    *some_value = 12.34;
    pthread_exit(some_value);
}

int main() {
    pthread_t child;
    if (pthread_create(&child, NULL, do_job, NULL) == 0) {
        double *return_value;
        if (pthread_join(child, &return_value) != 0) {
            printf("Can't join thread\n");
            return 1;
        }
        printf("Thread's return value: %lf\n", *return_value);
        for (int i = 0; i < 10; ++i) {
            printf("Hello from old thread!\n");
        }
    } else {
        printf("Can't create thread\n");
        return 1;
    }
    return 0;
}