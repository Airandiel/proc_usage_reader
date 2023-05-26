#ifndef QUEUE_H
#define QUEUE_H
#include <pthread.h>
#include <stdbool.h>

#include "logger.h"

typedef struct node {
    void* data;
    struct node* next;
} Node;

typedef struct Queue {
    Node* head;
    Node* tail;
    pthread_mutex_t mutex;
    LoggerThread* logger;
} Queue;

Queue* queue_init(LoggerThread* logger);
void queue_enqueue(Queue* queue, void* data);
bool queue_dequeue(Queue* queue, void** data);
bool queue_is_empty(Queue* queue);
void queue_destroy(Queue* queue);

#endif /* QUEUE_H */
