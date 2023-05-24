#ifndef QUEUE_H
#define QUEUE_H
#include <pthread.h>

#include "logger.h"

typedef struct node {
    void* data;
    struct node* next;
} Node;

typedef struct queue {
    Node* head;
    Node* tail;
    pthread_mutex_t mutex;
    struct logger_thread* logger;
} Queue;

Queue* queue_init(struct logger_thread* logger);
void queue_enqueue(Queue* queue, void* data);
int queue_dequeue(Queue* queue, void** data);
void queue_destroy(Queue* queue);

#endif /* QUEUE_H */
