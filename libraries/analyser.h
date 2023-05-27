#ifndef ANALYSER_H
#define ANALYSER_H

#include <pthread.h>

#include "queue.h"

typedef struct {
    Queue* queue;
    Queue* printer_queue;
    LoggerThread* logger;
    int running;
    pthread_t thread;
} AnalyserData;

typedef struct {
    double usage;
    int cpu_no;
} CpuUsageData;

typedef struct {
    unsigned long long total_idle, total;
} CpuValues;

CpuUsageData* calculate_cpu_usage(const char* cpu_stats_line, bool first_line,
                                  CpuValues* prev_val);
void* analyser_thread_func(void* arg);
AnalyserData* analyser_create(Queue* queue, Queue* printer_queue,
                              LoggerThread* logger);
pthread_t analyser_start(AnalyserData* analyser_data);
void analyser_destroy(AnalyserData* analyser_data);

#endif /* ANALYSER_H */