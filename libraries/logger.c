#include "logger.h"

#include <time.h>

void* logger_thread_func(void* arg) {
    LoggerThread* logger = (LoggerThread*)arg;
    struct log_message* log_msg;

    while (logger->running) {
        struct timespec timeout;
        clock_gettime(CLOCK_REALTIME, &timeout);
        timeout.tv_sec += 2;

        pthread_mutex_lock(&logger->mutex);
        // pthread_cond_wait(&logger->cond, &logger->mutex);  // wait for signal
        // without timeout
        int result =
            pthread_cond_timedwait(&logger->cond, &logger->mutex, &timeout);

        // if (result == ETIMEDOUT) {
        //     printf("Timed out waiting for condition variable\n");
        // } else if (result == 0) {
        //     printf("Condition variable signaled\n");
        // } else {
        //     printf("Error waiting for condition variable: %d\n", result);
        // }

        while (logger->running && !TAILQ_EMPTY(&logger->log_queue)) {
            pthread_mutex_lock(&logger->mutex_queue);

            log_msg = TAILQ_FIRST(&logger->log_queue);
            TAILQ_REMOVE(&logger->log_queue, log_msg, entries);
            pthread_mutex_unlock(&logger->mutex_queue);

            time_t now = time(NULL);
            char time_str[MAX_LOG_LENGTH];

            strftime(time_str, sizeof(time_str), "[%Y-%m-%d %H:%M:%S]",
                     localtime(&now));
            fprintf(logger->log_file, "%s [%s] %s\n", time_str,
                    log_msg->module_name, log_msg->message);
            fflush(logger->log_file);
        }
        pthread_mutex_unlock(&logger->mutex);
    }

    free(log_msg);
    pthread_mutex_unlock(&logger->mutex);
    pthread_exit(NULL);
}

LoggerThread* logger_create_thread(const char* log_file_path) {
    LoggerThread* logger = (LoggerThread*)malloc(sizeof(LoggerThread));

    logger->log_file = fopen(log_file_path, "a");
    if (!logger->log_file) {
        printf("Error: could not open log file %s\n", log_file_path);
        free(logger);
        return NULL;
    }

    TAILQ_INIT(&logger->log_queue);

    pthread_mutex_init(&logger->mutex, NULL);
    pthread_mutex_init(&logger->mutex_queue, NULL);
    pthread_cond_init(&logger->cond, NULL);

    logger->running = true;
    // increasing the stack size of the thread, not used
    // pthread_attr_t attr;
    // pthread_attr_init(&attr);
    // pthread_attr_setstacksize(&attr, 65536);
    int result = pthread_create(&logger->thread, NULL, logger_thread_func,
                                (void*)logger);
    if (result) {
        printf("Error: couldn't create logger thread\n");
        logger_destroy_thread(logger);
        return NULL;
    }

    return logger;
}

void logger_destroy_thread(LoggerThread* logger) {
    logger->running = false;
    pthread_cond_signal(&logger->cond);
    pthread_join(logger->thread, NULL);

    pthread_mutex_destroy(&logger->mutex);
    pthread_mutex_destroy(&logger->mutex_queue);
    pthread_cond_destroy(&logger->cond);

    fclose(logger->log_file);

    free(logger);
}

void log_message(LoggerThread* logger, const char* module_name,
                 const char* message) {
    struct log_message* log_msg =
        (struct log_message*)malloc(sizeof(struct log_message));
    strcpy(log_msg->module_name, module_name);
    strcpy(log_msg->message, message);

    pthread_mutex_lock(&logger->mutex_queue);
    TAILQ_INSERT_TAIL(&logger->log_queue, log_msg, entries);
    pthread_mutex_unlock(&logger->mutex_queue);

    pthread_mutex_lock(&logger->mutex);
    pthread_cond_signal(&logger->cond);
    pthread_mutex_unlock(&logger->mutex);
}
