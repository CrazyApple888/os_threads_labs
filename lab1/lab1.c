#include <stdio.h>
#include <pthread.h>

void* do_job(void *_) {
    for (int i = 0; i < 10; ++i) {
        printf("Hello from new thread!\n");
    }
    return (void *)1;
}

int main() {
    pthread_t child;
    if (pthread_create(&child, NULL, do_job, NULL) == 0) {
        for (int i = 0; i < 10; ++i) {
            printf("Hello from old thread!\n");
        }
    } else {
        printf("Can't create thread\n");
        return 1;
    }
    
    return 0;
}
