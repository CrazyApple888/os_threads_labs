#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define COEFFICIENT 1

pthread_barrier_t global_barrier;

typedef struct Block {
    unsigned int sleep_time;
    pthread_t *thread;
    char *string;
} Block;

void *sort_block(void *block) {
    pthread_barrier_wait(&global_barrier);
    Block *mBlock = (Block *) block;
    sleep(mBlock->sleep_time);
    printf("%s", mBlock->string);
    return 0;
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
    if (argc < 2) {
        printf("Not enough arguments\n");
    } else if (argc > 2) {
        printf("Too much arguments\n");
    }
    //TODO check
    int number_of_strings = atoi(argv[1]);
    ssize_t symbols_read;
    char **string_buffer = (char **) malloc(sizeof(char *) * number_of_strings);
    if (string_buffer == NULL) {
        printf("Can't allocate memory\n");
        return 1;
    }
    Block **block_buffer = (Block **) malloc(sizeof(Block *) * number_of_strings);
    if (block_buffer == NULL) {
        free(string_buffer);
        printf("Can't allocate memory\n");
        return 1;
    }
    for (int i = 0; i < number_of_strings; ++i) {
        if ((string_buffer[i] = (char*) malloc(sizeof(char) * BUFSIZ)) == NULL) {
            destroyStringBuffer(string_buffer, i);
            free(block_buffer);
            printf("Can't allocate memory for string buffer element %d\n", i);
            return 1;
        }
    }
    for (int i = 0; i < number_of_strings; ++i) {
        if ((symbols_read = read(STDIN_FILENO, string_buffer[i], BUFSIZ)) < -1) {
            printf("Error: can't save string. Terminating program...\n");
            if (destroyStringBuffer(string_buffer, number_of_strings) != 0) {
                printf("Can't clean string buffer...\n");
            }
            return 1;
        }
        block_buffer[i] = newBlock(string_buffer[i], symbols_read);
    }

    //------------------------------------------------------------------
    pthread_barrier_init(&global_barrier, NULL, number_of_strings + 1);
    for (int i = 0; i < number_of_strings; ++i) {
        if (pthread_create(block_buffer[i]->thread, NULL, sort_block, block_buffer[i]) != 0) {
            printf("ERROR: can't create thread\n");
            destroyBlockBuffer(block_buffer, i);
            destroyStringBuffer(string_buffer, number_of_strings);
            return 1;
        }
    }
    pthread_barrier_wait(&global_barrier);
    printf("Starting sort\n");
    pthread_barrier_destroy(&global_barrier);
    for (int i = 0; i < number_of_strings; ++i) {
        if (pthread_join(*(block_buffer[i]->thread), NULL) != 0) {
            printf("Can't join thread\n");
            break;
        }
    }
    destroyBlockBuffer(block_buffer, number_of_strings);
    destroyStringBuffer(string_buffer, number_of_strings);
    return 0;
}