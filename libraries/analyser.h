#ifndef ANALYSER_H
#define ANALYSER_H

#include <pthread.h>

#include "queue.h"

#define NUM_CPU_STATS 4
#define MAX_LINE_LENGTH 256

typedef struct {
    Queue* queue;
    LoggerThread* logger;
    int running;
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
AnalyserData* analyser_create(Queue* queue, LoggerThread* logger);
pthread_t analyser_start(AnalyserData* analyser_data);
void analyser_stop(AnalyserData* analyser_data, pthread_t thread);

#endif /* ANALYSER_H */