#ifndef LOGGER_H
#define LOGGER_H

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>

#define MAX_LOG_LENGTH 256  // maximum length of log message
#define TIMEOUT 2           // timeout in seconds

struct log_message {
    char module_name[MAX_LOG_LENGTH];
    char message[MAX_LOG_LENGTH];
    TAILQ_ENTRY(log_message) entries;
};

typedef struct {
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_mutex_t mutex_queue;
    pthread_cond_t cond;
    TAILQ_HEAD(log_message_queue, log_message) log_queue;
    FILE* log_file;
    bool running;
} LoggerThread;

LoggerThread* logger_create_thread(const char* log_file_path);

void logger_destroy_thread(LoggerThread* logger);

void log_message(LoggerThread* logger, const char* module_name,
                 const char* message);

#endif
