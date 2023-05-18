// #ifndef LOGGER_THREAD_H
// #define LOGGER_THREAD_H

// #include <pthread.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <sys/queue.h>
// #include <time.h>
// #include <unistd.h>

// /* Define the maximum size of the log file path */
// #define MAX_LOG_FILE_PATH_SIZE 256

// /* Define the maximum size of a log message */
// #define MAX_LOG_MESSAGE_SIZE 1024

// /* Define the maximum size of the module name */
// #define MAX_MODULE_NAME_SIZE 32

// /* Define the structure for a log message */
// struct log_message {
//     TAILQ_ENTRY(log_message) entries;
//     char message[MAX_LOG_MESSAGE_SIZE];
//     char module_name[MAX_MODULE_NAME_SIZE];
//     time_t timestamp;
// };

// /* Define the structure for the logger thread */
// struct logger_thread {
//     char log_file_path[MAX_LOG_FILE_PATH_SIZE];
//     int keep_running;
//     pthread_t thread;
//     pthread_mutex_t mutex;
//     TAILQ_HEAD(log_queue, log_message) queue;
// };

// /* Define the function prototypes */
// void log_message(struct logger_thread *logger, const char *module_name,
//                  const char *format, ...);
// struct logger_thread *create_logger_thread(const char *log_file_path);
// void destroy_logger_thread(struct logger_thread *logger);

// #endif /* LOGGER_THREAD_H */

#ifndef LOGGER_H
#define LOGGER_H

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>

#include "watchdog.h"

#define MAX_LOG_LENGTH 256  // maximum length of log message
#define TIMEOUT 2           // timeout in seconds

struct log_message {
    char module_name[MAX_LOG_LENGTH];
    char message[MAX_LOG_LENGTH];
    TAILQ_ENTRY(log_message) entries;
};

struct logger_thread {
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_mutex_t mutex_queue;
    pthread_cond_t cond;
    TAILQ_HEAD(log_message_queue, log_message) log_queue;
    FILE* log_file;
    bool running;
    struct watchdog* watchdog;
};

struct logger_thread* logger_create_thread(const char* log_file_path);

void logger_destroy_thread(struct logger_thread* logger);

void log_message(struct logger_thread* logger, const char* module_name,
                 const char* message);

void logger_add_watchdog(struct logger_thread* logger,
                         struct watchdog* watchdog);

#endif
