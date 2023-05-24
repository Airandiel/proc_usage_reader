#ifndef READER_H
#define READER_H

#include <pthread.h>

#include "queue.h"

#define MAX_CPU_STATS 10
#define STATS_INTERVAL 1
#define KICK_INTERVAL 2

typedef struct reader {
    const char* filename;
    Queue* queue;
    pthread_t thread;
    struct logger_thread* logger;
    int running;
} Reader;

typedef struct reader_data {
    char cpu_stats[MAX_CPU_STATS][256];
} ReaderData;

Reader* reader_create(const char* filename, Queue* queue,
                      struct logger_thread* logger);
pthread_t reader_start(Reader* reader);
void reader_destroy(Reader* reader);
void* reader_thread_func(void* arg);

#endif /* READER_H */