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

pthread_mutex_t mutex;
int isCompleted = FALSE;

typedef struct Node {
    char *string;
    struct Node *next;
} Node;

Node *head;

void freeList() {
    Node *tmp;
    for (Node *node = head; node != NULL; node = tmp) {
        if (NULL != node->string) {
            free(node->string);
        }
        tmp = node->next;
        free(node);
    }
}

int destroyMutex() {
    if (0 != pthread_mutex_destroy(&mutex)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void fatalExit() {
    if (EXIT_FAILURE == destroyMutex()) {
        printf("Can't destroy mutex\n");
    }
    freeList();
    exit(EXIT_FAILURE);
}

Node *newNode() {
    Node *node = (Node *) malloc(sizeof(Node));
    if (NULL == node) {
        fatalExit();
    }
    node->string = NULL;
    node->next = NULL;

    return node;
}

void lockMutex() {
    if (0 != pthread_mutex_lock(&mutex)) {
        fatalExit();
    }
}

void unlockMutex() {
    if (0 != pthread_mutex_unlock(&mutex)) {
        fatalExit();
    }
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
    lockMutex();
    if (NULL == head->string) {
        putString(head, string);
        unlockMutex();
        return;
    }
    Node *node = newNode();
    putString(node, string);
    node->next = head;
    head = node;
    unlockMutex();
}

void printList() {
    printf("----------LIST----------\n");
    lockMutex();
    for (Node *node = head; NULL != node; node = node->next) {
        if (NULL == node->string) {
            continue;
        }
        printf("%s", node->string);
    }
    unlockMutex();
    printf("--------END LIST--------\n");
}

void initMutex() {
    if (0 != pthread_mutex_init(&mutex, NULL)) {
        fatalExit();
    }
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

void swapNodes(Node *prevOne, Node *one, Node *prevTwo, Node *two) {
    if (one != NULL && two != NULL) {

        if (prevOne != NULL) {
            prevOne->next = two;
        } else {
            head = two;
        }

        if (prevTwo != NULL) {
            prevTwo->next = one;
        } else {
            head = one;
        }

        Node *tmp = one->next;
        one->next = two->next;
        two->next = tmp;
    }
}

void *sort(void *_) {
    while (TRUE) {
        if (0 != sleep(SLEEP_TIME)) {
            fatalExit();
        }
        lockMutex();
        if (TRUE == isCompleted) {
            unlockMutex();
            break;
        }

        Node *prevOne = NULL;
        for (Node *node = head; NULL != node; node = node->next) {
            Node *prevTwo = node;
            for (Node *innerNode = node->next; NULL != innerNode; innerNode = innerNode->next) {
                if (compareStrings(node->string, innerNode->string) > 0) {
                    swapNodes(prevOne, node, prevTwo, innerNode);
                    Node *tmp = innerNode;
                    innerNode = node;
                    node = tmp;
                }
                prevTwo = innerNode;
            }
            prevOne = node;
        }
        unlockMutex();
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
    lockMutex();
    isCompleted = TRUE;
    unlockMutex();
}

int main() {
    pthread_t sorter;
    initMutex();
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
