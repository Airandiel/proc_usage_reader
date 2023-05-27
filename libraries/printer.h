#ifndef PRINTER_H
#define PRINTER_H

#include <pthread.h>

#include "logger.h"
#include "queue.h"

typedef struct {
    Queue* queue;
    LoggerThread* logger;
    pthread_t thread;
    bool running;
} Printer;

typedef struct {
    double* usage;
    int* cpu;
    int no_lines;
} PrintCpuData;

void print_usage(const PrintCpuData* cpu_usage);
void* printer_thread_func(void* arg);
Printer* printer_create(Queue* queue, LoggerThread* logger);
pthread_t printer_start(Printer* printer);
void printer_destroy(Printer* printer);
PrintCpuData* print_cpu_data_init();
#endif /* PRINTER_H */