#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TRUE 1
#define FALSE 0
#define SLEEP_TIME 5
#define SHOW_STRINGS_SYMBOL "\n"
#define BUFFER_SIZE 80
#define STOP_WORD "STOP\n"

int isCompleted = FALSE;

typedef struct Node {
    char *string;
    struct Node *next;
    pthread_mutex_t mutex;
} Node;

Node *head;

void freeList() {
    Node* tmp;
    for (Node *node = head; node != NULL; node = tmp) {
        if (NULL != node->string) {
            free(node->string);
        }
        pthread_mutex_destroy(&(node->mutex));
        tmp = node->next;
        free(node);
    }
}

void fatalExit() {
    freeList();
    exit(EXIT_FAILURE);
}

void lockMutex(pthread_mutex_t *mutex) {
    if (0 != pthread_mutex_lock(mutex)) {
        fatalExit();
    }
}

void unlockMutex(pthread_mutex_t *mutex) {
    if (0 != pthread_mutex_unlock(mutex)) {
        fatalExit();
    }
}

Node *newNode() {
    Node *node = (Node *) malloc(sizeof(Node));
    if (NULL == node) {
        fatalExit();
    }
    node->string = NULL;
    node->next = NULL;
    if (0 != pthread_mutex_init(&(node->mutex), NULL)) {
        fatalExit();
    }

    return node;
}

void putString(Node *node, char *string) {
    size_t length = strlen(string);
    if ('\n' != string[length - 1]) {
        string[length] = '\n';
    }
    node->string = (char *) malloc(sizeof(char) * length);
    strncpy(node->string, string, length + 1);
}

void pushFront(char *string) {
    if (NULL == string || NULL == head) {
        fatalExit();
    }
    lockMutex(&(head->mutex));
    if (NULL == head->string) {
        putString(head, string);
        unlockMutex(&(head->mutex));
        return;
    }
    Node *node = newNode();
    putString(node, string);
    node->next = head;
    head = node;
    unlockMutex(&(head->next->mutex));
}

void printList() {
    printf("----------LIST----------\n");
    for (Node *node = head; NULL != node; node = node->next) {
        lockMutex(&(node->mutex));
        if (NULL == node->string) {
            unlockMutex(&(node->mutex));
            continue;
        }
        printf("%s", node->string);
        unlockMutex(&(node->mutex));
    }
    printf("--------END LIST--------\n");
}

void swap(char **first, char **second) {
    char *tmp = *first;
    *first = *second;
    *second = tmp;
}

int compareStrings(char *first, char *second) {
    if (NULL == first) {
        return 1;
    }
    if (NULL == second) {
        return -1;
    }
    return strcmp(first, second);
}

//TODO rewrite me
void *sort(void *_) {
    while (TRUE) {
        if (0 != sleep(SLEEP_TIME)) {
            fatalExit();
        }
        if (TRUE == isCompleted) {
            break;
        }

        Node *first = NULL;
        Node *second = NULL;
        Node *third = NULL;
        lockMutex(&head->mutex);
        for(first = head; NULL != first->next; ) {
            //lockMutex(&first->mutex);
            for(second = first->next; NULL != second; second = second->next) {
                lockMutex(&second->mutex);
                if (compareStrings(first->string, second->string) > 0) {
                    swap(&first->string, &second->string);
                }
                unlockMutex(&second->mutex);
            }
            if (NULL != first->next) {
                third = first->next;
                lockMutex(&third->mutex);
            }
            unlockMutex(&first->mutex);
            first = third;
        }
        unlockMutex(&first->mutex);
    }

    return (void *) EXIT_SUCCESS;
}

void readStrings() {
    char buffer[BUFFER_SIZE + 1];

    while (fgets(buffer, BUFFER_SIZE + 1, stdin)) {
        if (0 == strcmp(buffer, STOP_WORD)) {
            printf("Stopping program...\n");
            break;
        }
        if (0 == strcmp(buffer, SHOW_STRINGS_SYMBOL)) {
            printList();
        } else {
            pushFront(buffer);
        }
        fflush(stdout);
    }
    isCompleted = TRUE;
}

int main() {
    pthread_t sorter;
    head = newNode();
    if (0 != pthread_create(&sorter, NULL, sort, NULL)) {
        fatalExit();
    }
    printf("Print STOP to stop program :)\n");
    readStrings();

    pthread_join(sorter, NULL);

    freeList();
    return EXIT_SUCCESS;
}
