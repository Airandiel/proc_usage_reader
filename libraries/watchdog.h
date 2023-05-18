#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <pthread.h>
#include <stdbool.h>

#define MAX_THREADS 10
#define TIMEOUT 2

struct thread_info {
    int type;
    char name[20];
    int thread_id;
    bool is_running;
};

struct watchdog {
    struct thread_info threads[MAX_THREADS];
    struct logger_thread* logger;
    int num_threads;
    pthread_mutex_t mutex;
    bool running;
    pthread_t thread;
    pthread_cond_t stop_cond;
};

void* watchdog_create(struct logger_thread* logger, pthread_cond_t stop_cond);

void watchdog_add_thread(void* watchdog, int type, char name[20],
                         int thread_id);
void watchdog_remove_thread(void* watchdog, int thread_id);
// void destroy_watchdog(void* watchdog);
void watchdog_kick(void* watchdog, int thread_id);

#endif
