#include "analyser.h"

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "logger.h"
#include "printer.h"
#include "queue.h"
#include "reader.h"

CpuUsageData* calculate_cpu_usage(const char* cpu_stats_line, bool first_line,
                                  CpuValues* prev_val) {
    CpuUsageData* cpu_usage = (CpuUsageData*)malloc(sizeof(CpuUsageData));
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal,
        guest, guest_nice;
    int cpu_number;

    if (first_line) {
        sscanf(cpu_stats_line,
               "cpu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu", &user,
               &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest,
               &guest_nice);
        cpu_usage->cpu_no = -1;
    } else {
        sscanf(cpu_stats_line,
               "cpu%d %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
               &cpu_number, &user, &nice, &system, &idle, &iowait, &irq,
               &softirq, &steal, &guest, &guest_nice);
        cpu_usage->cpu_no = cpu_number + 1;
    }

    unsigned long long non_idle = user + nice + system + irq + softirq + steal;
    if (non_idle > 0) {
        unsigned long long total_idle = idle + iowait;
        unsigned long long total = total_idle + non_idle;
        unsigned long long total_diff = total - prev_val->total;
        unsigned long long idle_diff = total_idle - prev_val->total_idle;
        if (total_diff - idle_diff != 0) {
            cpu_usage->usage =
                (double)(total_diff - idle_diff) / total_diff * 100;
        } else {
            cpu_usage->usage = 0;
        }
        prev_val->total = total;
        prev_val->total_idle = total_idle;
    } else {
        prev_val->total = 0;
        prev_val->total_idle = 0;
        cpu_usage->usage = 0.;
    }

    return cpu_usage;
}

void* analyser_thread_func(void* arg) {
    AnalyserData* analyser_data = (AnalyserData*)arg;
    CpuValues** cpu_val_array =
        (CpuValues**)malloc(MAX_CPU_STATS * sizeof(CpuValues**));
    PrintCpuData* cpu_printer_data = print_cpu_data_init();
    for (int i = 0; i < MAX_CPU_STATS; i++) {
        cpu_val_array[i] = (CpuValues*)malloc(sizeof(CpuValues*));
        memset(cpu_val_array[i], 0, sizeof(CpuValues));
    }
    ReaderData* cpu_stats;

    while (analyser_data->running) {
        while (!queue_is_empty(analyser_data->queue) &&
               queue_dequeue(analyser_data->queue, (void**)&cpu_stats)) {
            CpuUsageData* cpu_usage = calculate_cpu_usage(
                cpu_stats->cpu_stats[0], true, cpu_val_array[0]);
            cpu_printer_data->cpu[0] = -1;
            cpu_printer_data->usage[0] = cpu_usage->usage;

            // printf("Total CPU Usage : %.2f%%\n", cpu_usage->usage);
            for (int i = 1; i < cpu_stats->no_lines; i++) {
                cpu_usage = calculate_cpu_usage(cpu_stats->cpu_stats[i], false,
                                                cpu_val_array[i]);
                cpu_printer_data->cpu[i] = cpu_usage->cpu_no;
                cpu_printer_data->usage[i] = cpu_usage->usage;
                // printf("CPU%d Usage: %.2f%%\n", cpu_usage->cpu_no,
                //    cpu_usage->usage);
            }
            cpu_printer_data->no_lines = cpu_stats->no_lines;
            queue_sig_enqueue(analyser_data->printer_queue,
                              (void*)cpu_printer_data);
        }
        sleep(1);
    }

    for (int i = 0; i < MAX_CPU_STATS; i++) {
        free(cpu_val_array[i]);
    }
    free(cpu_val_array);
    free(cpu_stats);

    pthread_exit(NULL);
}

AnalyserData* analyser_create(Queue* queue, Queue* printer_queue,
                              LoggerThread* logger) {
    AnalyserData* analyser_data = (AnalyserData*)malloc(sizeof(AnalyserData));
    analyser_data->queue = queue;
    analyser_data->printer_queue = printer_queue;
    analyser_data->logger = logger;
    analyser_data->running = true;
    return analyser_data;
}

pthread_t analyser_start(AnalyserData* analyser_data) {
    pthread_t thread;
    int result =
        pthread_create(&thread, NULL, analyser_thread_func, analyser_data);
    if (result != 0) {
        log_message(analyser_data->logger, "Analyser",
                    "ERROR: failed to create analyser thread\n");
        free(analyser_data);
        return 0;
    }
    analyser_data->thread = thread;
    return thread;
}

void analyser_destroy(AnalyserData* analyser_data) {
    analyser_data->running = false;
    pthread_cancel(analyser_data->thread);
    pthread_join(analyser_data->thread, NULL);
    free(analyser_data);
}
