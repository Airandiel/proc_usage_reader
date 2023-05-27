#include "queue.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include "logger.h"

Queue* queue_init(LoggerThread* logger) {
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    queue->head = NULL;
    queue->tail = NULL;
    pthread_mutex_init(&queue->mutex, NULL);
    queue->logger = logger;
    pthread_mutex_init(&queue->cond_mutex, NULL);
    pthread_cond_init(&queue->cond, NULL);
    return queue;
}

bool queue_enqueue(Queue* queue, void* data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        log_message(queue->logger, "Queue",
                    "ERROR: Failed to allocate memory for new node");
        return false;
    }

    newNode->data = data;
    newNode->next = NULL;

    pthread_mutex_lock(&queue->mutex);

    if (queue->tail == NULL) {
        // queue is empty
        queue->head = newNode;
        queue->tail = newNode;
    } else {
        // queue has elements
        queue->tail->next = newNode;
        queue->tail = newNode;
    }
    pthread_mutex_unlock(&queue->mutex);
    log_message(queue->logger, "Queue", "INFO: Enqueued data");
    return true;
}

bool queue_dequeue(Queue* queue, void** data) {
    pthread_mutex_lock(&queue->mutex);

    if (queue->head == NULL) {
        // queue is empty
        pthread_mutex_unlock(&queue->mutex);
        return false;
    }

    Node* temp = queue->head;
    *data = temp->data;

    if (queue->head == queue->tail) {
        // queue had only one element
        queue->head = NULL;
        queue->tail = NULL;
    } else {
        // queue has more than one element
        queue->head = queue->head->next;
    }

    free(temp);

    pthread_mutex_unlock(&queue->mutex);
    log_message(queue->logger, "Queue", "INFO: Dequeued data");

    return true;
}

bool queue_sig_dequeue(Queue* queue, void** data) {
    struct timespec timeout;
    struct timeval now;

    // Calculate the absolute timeout value
    // gettimeofday(&now, NULL);
    // timeout.tv_sec = now.tv_sec + 2;

    clock_gettime(CLOCK_REALTIME, &timeout);
    timeout.tv_sec += 2;

    pthread_mutex_lock(&queue->cond_mutex);
    int result =
        pthread_cond_timedwait(&queue->cond, &queue->cond_mutex, &timeout);
    bool dequeue_res = false;
    if (result == 0) {
        while (!queue_is_empty(queue)) {
            dequeue_res = queue_dequeue(queue, data);
        }
    }
    pthread_mutex_unlock(&queue->cond_mutex);
    return dequeue_res;
}

bool queue_sig_enqueue(Queue* queue, void* data) {
    bool result = queue_enqueue(queue, data);
    if (result) {
        pthread_cond_signal(&queue->cond);
        printf("Enqueueuing and sending signal\n");
        return true;
    } else {
        return false;
    }
}

bool queue_is_empty(Queue* queue) {
    pthread_mutex_lock(&queue->mutex);
    bool isEmpty = (queue->head == NULL);
    pthread_mutex_unlock(&queue->mutex);

    return isEmpty;
}

void queue_destroy(Queue* queue) {
    pthread_cond_signal(&queue->cond);
    pthread_mutex_lock(&queue->mutex);

    Node* current = queue->head;
    Node* next;

    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }

    queue->head = NULL;
    queue->tail = NULL;

    pthread_mutex_unlock(&queue->mutex);
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->cond);
}