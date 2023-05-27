#include "printer.h"

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "queue.h"
#include "reader.h"

void print_usage(const PrintCpuData* cpu_usage) {
    printf("\033[2J");    // clear screen
    printf("\033[1;1H");  // Move cursor to the top-left corner

    // Print the usages
    printf("Total CPU Usage : %.2f%%\n", cpu_usage->usage[0]);
    for (int i = 1; i < cpu_usage->no_lines; i++) {
        printf("CPU%d Usage: %.2f%%\n", cpu_usage->cpu[i], cpu_usage->usage[i]);
    }
    printf("\n");
}

void* printer_thread_func(void* arg) {
    Printer* printer = (Printer*)arg;
    PrintCpuData* cpu_data = (PrintCpuData*)malloc(sizeof(PrintCpuData));

    while (printer->running) {
        if (queue_sig_dequeue(printer->queue, (void**)&cpu_data)) {
            // Print the array elements
            print_usage(cpu_data);
        }
    }
    free(cpu_data->cpu);
    free(cpu_data->usage);
    free(cpu_data);

    pthread_exit(NULL);
}

Printer* printer_create(Queue* queue, LoggerThread* logger) {
    Printer* printer = (Printer*)malloc(sizeof(Printer));
    printer->queue = queue;
    printer->running = true;
    printer->logger = logger;
    return printer;
}

pthread_t printer_start(Printer* printer) {
    pthread_t thread;
    int result = pthread_create(&thread, NULL, printer_thread_func, printer);
    if (result != 0) {
        free(printer);
        return 0;
    }
    printer->thread = thread;
    return thread;
}

void printer_destroy(Printer* printer) {
    printer->running = false;
    pthread_cancel(printer->thread);
    pthread_join(printer->thread, NULL);
    free(printer);
}

PrintCpuData* print_cpu_data_init() {
    PrintCpuData* cpu_printer_data =
        (PrintCpuData*)malloc(sizeof(PrintCpuData));

    cpu_printer_data->cpu = (int*)malloc(MAX_CPU_STATS * sizeof(int));
    cpu_printer_data->usage = (double*)malloc(MAX_CPU_STATS * sizeof(double));
    cpu_printer_data->no_lines = 0;
    memset(cpu_printer_data->cpu, 0, MAX_CPU_STATS * sizeof(int));
    memset(cpu_printer_data->usage, 0, MAX_CPU_STATS * sizeof(double));
    return cpu_printer_data;
}
