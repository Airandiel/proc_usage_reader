#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <time.h>
#include <unistd.h>

#include "logger.h"
#include "reader.h"

volatile sig_atomic_t done = 0;

volatile bool flag_running = true;
struct logger_thread* logger;
struct watchdog* watchdog;
pthread_cond_t stop_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t stop_mutex = PTHREAD_MUTEX_INITIALIZER;
Queue* queue;

void term(int signum) {
    done = 1;
    flag_running = 0;
    // pthread_cond_broadcast(&log_sent);
}

void process_cpu_stats(const ReaderData* cpu_stats) {
    printf("Received CPU stats: %s\n", cpu_stats->cpu_stats[0]);
}

void* printing_thread(char* message) {
    char* module = "PRINTING THREAD";
    while (flag_running) {
        printf("Sending to log: %s\n", message);
        log_message(logger, module, message);
        int t = sleep(1);
    }
    printf("%s", message);
}

int main() {
    // struct sigaction action;
    // memset(&action, 0, sizeof(struct sigaction));
    // action.sa_handler = term;
    // sigaction(SIGTERM, &action, NULL);
    // sigaction(SIGQUIT, &action, NULL);

    // pthread_t thread_id;
    // pthread_t thread_id2;
    char* message_to_log = "Log_message";
    logger = logger_create_thread("log.txt");

    // watchdog = watchdog_create(logger, stop_cond);
    // logger_add_watchdog(logger, watchdog);

    queue = queue_init(logger);
    Reader* reader = reader_create("/proc/stat", queue, logger);
    pthread_t reader_thread = reader_start(reader);
    int counter = 0;
    while (counter < 5) {
        int t = sleep(1);
        ReaderData* cpu_stats = malloc(sizeof(ReaderData));
        queue_dequeue(queue, (void**)&cpu_stats);
        if (cpu_stats != NULL) {
            process_cpu_stats(cpu_stats);
        }
        counter++;
        printf("Waiting ended %d\n", counter);
    }

    reader->running = 0;

    pthread_join(reader_thread, NULL);

    // printf("Before Thread\n");
    // pthread_create(&thread_id, NULL, printing_thread, message_to_log);
    // watchdog_add_thread(&watchdog, 0, "Logger", 1);
    // watchdog_add_thread(&watchdog, 1, "Printer", 2);

    // pthread_create(&thread_id2, NULL, logger_thread, &log_sent);
    // int loop = 0;
    // int counter = 0;
    // while (!done) {
    //     counter++;
    //     if (counter > 3) {
    //         term(0);
    //     }
    //     int t = sleep(1);
    // }
    // pthread_join(thread_id2, NULL);

    // printf("done.\n");

    // int t = sleep(5);

    struct timespec timeout;
    clock_gettime(CLOCK_REALTIME, &timeout);
    timeout.tv_sec += 10;

    reader_destroy(reader);
    queue_destroy(queue);

    pthread_mutex_lock(&stop_mutex);
    int result = pthread_cond_timedwait(&stop_cond, &stop_mutex, &timeout);
    pthread_mutex_unlock(&stop_mutex);

    pthread_cond_destroy(&stop_cond);
    pthread_mutex_destroy(&stop_mutex);

    logger_destroy_thread(logger);
    // pthread_join(thread_id, NULL);

    return 0;
}