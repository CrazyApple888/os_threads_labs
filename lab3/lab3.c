#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define NUMBER_OF_LINES 3
#define NUMBER_OF_THREADS 4

typedef struct ThreadContext {
    char** strings;
} ThreadContext;

void* do_job(void *context) {
    if (context == NULL) {
        printf("Error while passing args");
        return (void*)1;
    }
    char** strings = ((ThreadContext*)context)->strings;
    int index = 0;
    while (strings[index] != NULL) {
        printf("%s\n", strings[index]);
        index++;
    }
    return 0;
}

int destroyStrings(char** string_array, int size) {
    if (string_array == NULL || size <= 0) {
        return 1;
    }
    for (int i = 0; i < size; ++i) {
        free(string_array[i]);
    }
    free(string_array);
    return 0;
}

char** createStrings(int unique_id) {
    char** array;
    if ((array = (char**)malloc(sizeof(char*) * (NUMBER_OF_LINES + 1))) == NULL) {
        return NULL;
    }
    for (int i = 0; i < NUMBER_OF_LINES; ++i) {
        array[i] = (char*) malloc(sizeof(char) * BUFSIZ);
        if (array[i] == NULL) {
            destroyStrings(array, i);
            return NULL;
        }
        sprintf(array[i], "THIS IS ID %d", unique_id);
    }
    array[NUMBER_OF_LINES] = NULL;
    return array;
}

int main() {
    pthread_t threads[NUMBER_OF_THREADS];
    ThreadContext context[NUMBER_OF_THREADS];
    int successful_threads = NUMBER_OF_THREADS;

    for (int i = 0; i < NUMBER_OF_THREADS; ++i) {
        context[i].strings = createStrings(i);
        int result = pthread_create(&threads[i], NULL, do_job, (void *) &context[i]);
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
    for (int i = 0; i < NUMBER_OF_THREADS; ++i) {
        destroyStrings(context[i].strings, NUMBER_OF_LINES + 1);
    }
    return 0;
}
