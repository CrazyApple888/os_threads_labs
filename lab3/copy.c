#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#define NUMBER_OF_LINES 10
#define NUMBER_OF_THREADS 4


void* print_lines(void *lines) {
    for (int i = 0; i < NUMBER_OF_LINES; ++i) {

    }
}

char** createLineBuffer(int number_of_lines, char* template, int number_of_thread) {
    if (number_of_lines <= 0 || template == NULL) {
        return NULL;
    }
    char** buffer = (char**)malloc(sizeof(char) * number_of_lines);
    for (int i = 0; i < number_of_lines; ++i) {
        //TODO make function to count number of digits
        char *line = (char*)malloc(sizeof(char) * (strlen(template) + 50));
        sprintf(line, template, number_of_thread, i + 1);
        buffer[i] = line;
    }

    return buffer;
}

int main() {
    char* template = "This is thread %d, line %d\n";
    char** buffer;
    if ((buffer = createLineBuffer(NUMBER_OF_LINES, template, NUMBER_OF_THREADS)) == NULL) {
        return 1;
    }
    pthread_t threads[NUMBER_OF_THREADS];
    int number_of_successful_threads = 4;
    for (int i = 0; i < NUMBER_OF_THREADS; ++i) {
        int result = pthread_create(threads[i], NULL, &print_lines, (void*)buffer[i]);
        if (result != 0) {
            number_of_successful_threads = i;
        }
    }
    return 0;
}
