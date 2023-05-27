#include "reader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "queue.h"

Reader* reader_create(const char* filename, Queue* queue,
                      LoggerThread* logger) {
    Reader* reader = (Reader*)malloc(sizeof(Reader));
    if (reader == NULL) {
        log_message(logger, "Reader", "Error during reader memory allocation");
        return NULL;
    }

    reader->filename = "/proc/stat";
    reader->queue = queue;
    reader->logger = logger;
    reader->running = 0;

    return reader;
}

pthread_t reader_start(Reader* reader) {
    pthread_t reader_thread;

    int result =
        pthread_create(&reader_thread, NULL, reader_thread_func, reader);
    if (result != 0) {
        log_message(reader->logger, "Reader",
                    "Error: Failed to create reader thread\n");
        return 1;
    }
    reader->thread = reader_thread;
    return (reader_thread);
}

void reader_destroy(Reader* reader) {
    reader->running = false;
    pthread_cancel(reader->thread);
    pthread_join(reader->thread, NULL);
    free(reader);
}

void* reader_thread_func(void* arg) {
    Reader* reader = (Reader*)arg;

    FILE* file = fopen(reader->filename, "r");
    if (file == NULL) {
        log_message(reader->logger, "Reader",
                    "Error: Failed to open file for reading");
        return NULL;
    }

    reader->running = 1;

    char line[256];
    ReaderData cpu_data;
    int stats_counter = 0;

    while (reader->running) {
        sleep(STATS_INTERVAL);
        fseek(file, 0, SEEK_SET);
        memset(cpu_data.cpu_stats, 0, sizeof(cpu_data.cpu_stats));
        stats_counter = 0;

        // Read lines starting with "cpu" and store them in the ReaderData
        // object
        while (fgets(line, sizeof(line), file) != NULL) {
            if (strncmp(line, "cpu", 3) == 0) {
                strncpy(cpu_data.cpu_stats[stats_counter], line,
                        sizeof(cpu_data.cpu_stats[0]) - 1);
                stats_counter++;
                if (stats_counter >= MAX_CPU_STATS) {
                    break;
                }

                // char str[30];
                // sprintf(str, "Read line: CPU%d, %s", stats_counter, line);
                // log_message(reader->logger, "INFO: Reader", str);
            }
        }
        cpu_data.no_lines = stats_counter;
        // send data using queue
        queue_enqueue(reader->queue, (void*)&cpu_data);

        // Execute the watchdog_kick function every 2 seconds
        // if (stats_counter > 0) {
        //     watchdog_kick();
        // }
    }

    fclose(file);

    log_message(reader->logger, "Reader", "INFO: Finished reading file");

    return NULL;
}
