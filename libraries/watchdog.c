#include "watchdog.h"

#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "logger.h"

static void* watchdog_thread_func(void* arg) {
    struct watchdog* watchdog = (struct watchdog*)arg;

    while (watchdog->running) {
        time_t now = time(NULL);
        printf("[%s] Checking %d threads...\n", ctime(&now),
               watchdog->num_threads);
        char* str;
        sprintf(str, "[%s] Checking %d threads...\n", ctime(&now),
                watchdog->num_threads);
        log_message(watchdog->logger, "WATCHDOG", str);

        for (int i = 0; i < watchdog->num_threads; i++) {
            struct thread_info* info = &watchdog->threads[i];
            if (!info->is_running) {
                printf("Thread %s is not responding!\n", info->name);
                char* str;
                sprintf(
                    str,
                    "Thread %s, id: %d, is not responding! Closing program\n",
                    info->name, info->thread_id);
                log_message(watchdog->logger, "WATCHDOG", str);
                pthread_cond_broadcast(&watchdog->stop_cond);

            } else {
                info->is_running = false;
            }
        }

        sleep(TIMEOUT);
    }

    pthread_exit(NULL);
}

void* watchdog_create(LoggerThread* logger, pthread_cond_t stop_cond) {
    struct watchdog* watchdog =
        (struct watchdog*)malloc(sizeof(struct watchdog));

    watchdog->num_threads = 0;
    pthread_mutex_init(&watchdog->mutex, NULL);
    watchdog->stop_cond = stop_cond;
    watchdog->logger = logger;

    watchdog->running = true;
    int result = pthread_create(&watchdog->thread, NULL, watchdog_thread_func,
                                (void*)watchdog);
    if (result) {
        fprintf(stderr, "Error: could not create watchdog thread\n");
        // destroy_watchdog(watchdog);
        return NULL;
    }

    return watchdog;
}

void watchdog_add_thread(void* watchdog, int type, char name[20],
                         int thread_id) {
    struct watchdog* w = (struct watchdog*)watchdog;

    pthread_mutex_lock(&w->mutex);
    if (w->num_threads < MAX_THREADS) {
        struct thread_info* info = &w->threads[w->num_threads++];
        info->type = type;
        strcpy(info->name, name);
        info->thread_id = thread_id;
        printf("Thread %s added to watchdog\n", name);
    } else {
        fprintf(stderr, "Error: maximum number of threads exceeded\n");
    }
    pthread_mutex_unlock(&w->mutex);
}

void watchdog_remove_thread(void* watchdog, int thread_id) {
    struct watchdog* w = (struct watchdog*)watchdog;
    bool id_found = false;
    pthread_mutex_lock(&w->mutex);
    for (int i = 0; i < w->num_threads; i++) {
        struct thread_info* info = &w->threads[i];
        if (info->thread_id == thread_id) {
            id_found = true;
        }
        if (id_found) {
            if (i < (w->num_threads - 1)) {
                struct thread_info* info_next = &w->threads[i + 1];
                info->is_running = info_next->is_running;
                info->thread_id = info_next->thread_id;
                info->type = info_next->type;
                strcpy(info->name, info_next->name);
            } else {
                w->num_threads--;
            }
        }
    }
    if (!id_found) {
        fprintf(stderr, "Error: Thread id: %d, not found. \n", thread_id);
    }
    pthread_mutex_unlock(&w->mutex);
}

void watchdog_kick(void* watchdog, int thread_id) {
    struct watchdog* w = (struct watchdog*)watchdog;

    pthread_mutex_lock(&w->mutex);
    for (int i = 0; i < w->num_threads; i++) {
        struct thread_info* info = &w->threads[i];
        if (info->thread_id == thread_id) {
            info->is_running = true;
        }
    }
    pthread_mutex_unlock(&w->mutex);
}
