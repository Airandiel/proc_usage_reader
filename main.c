#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <time.h>
#include <unistd.h>

#include "analyser.h"
#include "logger.h"
#include "printer.h"
#include "reader.h"

volatile sig_atomic_t done = 0;

volatile bool flag_running = true;
LoggerThread* logger;
struct watchdog* watchdog;
pthread_cond_t stop_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t stop_mutex = PTHREAD_MUTEX_INITIALIZER;
Queue *queue, printer_queue;
Reader* reader;
AnalyserData* analyser;
Printer* printer;

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

void quit_program(int signal) {
    reader->running = 0;
    analyser_destroy(analyser);
    printer_destroy(printer);

    reader_destroy(reader);
    queue_destroy(queue);

    logger_destroy_thread(logger);
}

int main() {
    // struct sigaction action;
    // memset(&action, 0, sizeof(struct sigaction));
    // action.sa_handler = term;
    signal(SIGTERM, quit_program);
    signal(SIGQUIT, quit_program);

    // pthread_t thread_id;
    // pthread_t thread_id2;
    char* message_to_log = "Log_message";
    logger = logger_create_thread("log.txt");

    // watchdog = watchdog_create(logger, stop_cond);
    // logger_add_watchdog(logger, watchdog);

    Queue* queue = queue_init(logger);
    Queue* printer_queue = queue_init(logger);
    Reader* reader = reader_create("/proc/stat", queue, logger);
    AnalyserData* analyser = analyser_create(queue, printer_queue, logger);
    Printer* printer = printer_create(printer_queue, logger);

    pthread_t printer_thread = printer_start(printer);
    pthread_t reader_thread = reader_start(reader);
    pthread_t analyser_thread = analyser_start(analyser);
    int counter = 0;
    while (counter < 5) {
        int t = sleep(1);
        // ReaderData* cpu_stats = malloc(sizeof(ReaderData));
        // queue_dequeue(queue, (void**)&cpu_stats);
        // if (cpu_stats != NULL) {
        //     process_cpu_stats(cpu_stats);
        // }
        counter++;
        printf("Waiting ended %d\n", counter);
    }

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

    pthread_mutex_lock(&stop_mutex);
    int result = pthread_cond_timedwait(&stop_cond, &stop_mutex, &timeout);
    pthread_mutex_unlock(&stop_mutex);

    pthread_cond_destroy(&stop_cond);
    pthread_mutex_destroy(&stop_mutex);

    // pthread_join(thread_id, NULL);

    return 0;
}