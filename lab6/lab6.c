#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define COEFFICIENT 1
#define MAX_COUNT_OF_STRINGS 100
#define STOP_WORD "\n"

pthread_barrier_t global_barrier;

typedef struct Block {
    unsigned int sleep_time;
    pthread_t *thread;
    char *string;
} Block;

void *sort_block(void *block) {
    pthread_barrier_wait(&global_barrier);
    Block *mBlock = (Block *) block;
    if (sleep(mBlock->sleep_time)) {
        printf("Sleep terminated\n");
        return (void*)EXIT_FAILURE;
    }
    printf("%s", mBlock->string);
    return EXIT_SUCCESS;
}

Block *newBlock(char *string, ssize_t length) {
    Block *new = (Block *) malloc(sizeof(Block));
    if (new == NULL) {
        return NULL;
    }
    new->string = (char *) malloc(sizeof(char) * length);
    if (new->string == NULL) {
        free(new);
        return NULL;
    }
    new->thread = (pthread_t*) malloc(sizeof(pthread_t));
    if (new->thread == NULL) {
        free(new->string);
        free(new);
        return NULL;
    }
    new->sleep_time = COEFFICIENT * length;
    strcpy(new->string, string);
    return new;
}

int destroyBlockBuffer(Block **buffer, int size) {
    if (buffer == NULL || size <= 0) {
        return 1;
    }
    for (int i = 0; i < size; ++i) {
        if (buffer[i] == NULL) {
            continue;
        }
        free(buffer[i]->string);
        free(buffer[i]->thread);
    }
    free(buffer);
    return 0;
}

int destroyStringBuffer(char **buffer, int size) {
    if (buffer == NULL || size <= 0) {
        return 1;
    }
    for (int i = 0; i < size; ++i) {
        if (buffer[i] == NULL) {
            continue;
        }
        free(buffer[i]);
    }
    free(buffer);
    return 0;
}

int main(int argc, char *argv[]) {
    ssize_t symbols_read;
    char **string_buffer = (char **) malloc(sizeof(char *) * MAX_COUNT_OF_STRINGS);
    if (string_buffer == NULL) {
        printf("Can't allocate memory\n");
        return 1;
    }
    Block **block_buffer = (Block **) malloc(sizeof(Block *) * MAX_COUNT_OF_STRINGS);
    if (block_buffer == NULL) {
        free(string_buffer);
        printf("Can't allocate memory\n");
        return 1;
    }
    for (int i = 0; i < MAX_COUNT_OF_STRINGS; ++i) {
        if ((string_buffer[i] = (char*) malloc(sizeof(char) * BUFSIZ)) == NULL) {
            destroyStringBuffer(string_buffer, i);
            free(block_buffer);
            printf("Can't allocate memory for string buffer element %d\n", i);
            return 1;
        }
    }

    char string[BUFSIZ];
    int string_counter = MAX_COUNT_OF_STRINGS;
    int index;
    printf("Enter to stop inputting\n");
    while (string_counter > 0 && NULL != fgets(string, BUFSIZ - 1, stdin)) {
        if (0 == strcmp(string, STOP_WORD)) {
            break;
        }
        index = MAX_COUNT_OF_STRINGS - string_counter;
        strcpy(string_buffer[index], string);
        block_buffer[index] = newBlock(string_buffer[index], strlen(string));
        string_counter--;
        printf("%d\n", index);
    }

    int count_of_strings = MAX_COUNT_OF_STRINGS - string_counter;
    //------------------------------------------------------------------
    if (pthread_barrier_init(&global_barrier, NULL, count_of_strings + 1)) {
        destroyStringBuffer(string_buffer, MAX_COUNT_OF_STRINGS);
        return EXIT_FAILURE;
    }
    for (int i = 0; i < count_of_strings; ++i) {
        if (pthread_create(block_buffer[i]->thread, NULL, sort_block, block_buffer[i]) != 0) {
            printf("ERROR: can't create thread\n");
            destroyBlockBuffer(block_buffer, i);
            destroyStringBuffer(string_buffer, MAX_COUNT_OF_STRINGS);
            return EXIT_FAILURE;
        }
    }
    pthread_barrier_wait(&global_barrier);
    printf("Starting sort\n");
    for (int i = 0; i < count_of_strings; ++i) {
        if (pthread_join(*(block_buffer[i]->thread), NULL) != 0) {
            printf("Can't join thread\n");
            break;
        }
    }
    if (pthread_barrier_destroy(&global_barrier) != 0) {
        printf("DESTROY error\n");
        destroyStringBuffer(string_buffer, MAX_COUNT_OF_STRINGS);
        destroyBlockBuffer(block_buffer, MAX_COUNT_OF_STRINGS);
        return EXIT_FAILURE;
    }
    destroyBlockBuffer(block_buffer, MAX_COUNT_OF_STRINGS);
    destroyStringBuffer(string_buffer, MAX_COUNT_OF_STRINGS);
    return 0;
}