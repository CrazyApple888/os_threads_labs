#include <stdio.h>
#include <pthread.h>

#define NUMBER_OF_LINES 3
#define NUMBER_OF_THREADS 4

typedef struct ThreadContext {
    int thread_number;
} ThreadContext;

void* do_job(void *ctx) {
    if (ctx == NULL) {
        printf("Error while passing args");
        return (void*)1;
    }
    for (int i = 0; i < NUMBER_OF_LINES; ++i) {
        printf("This is thread %d, line %d\n", ((ThreadContext*)ctx)->thread_number, i);
    }
    return 0;
}

int main() {
    pthread_t threads[NUMBER_OF_THREADS];
    ThreadContext ctx[NUMBER_OF_THREADS];
    int successful_threads = NUMBER_OF_THREADS;

    for (int i = 0; i < NUMBER_OF_THREADS; ++i) {
        ctx[i].thread_number = i;
        int result = pthread_create(&threads[i], NULL, do_job, (void *) &ctx[i]);
        if (result != 0) {
            successful_threads = i;
            break;
        }
    }
    printf("MAIN THREAD: %d threads are working\n", successful_threads);
    for (int i = 0; i < successful_threads; ++i) {
        int result = pthread_join(threads[i], NULL);
        if (result != 0) {
            successful_threads--;
        }
    }
    printf("MAIN THREAD: %d threads successfully joined\n", successful_threads);
    return 0;
}
