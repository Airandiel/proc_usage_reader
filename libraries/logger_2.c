#include "logger.h"

#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <time.h>
#include <unistd.h>

volatile bool flag_running = true;

void add_to_queue(char* mes) {
    struct entry* elem;
    elem = malloc(sizeof(struct entry));
    if (elem) {
        elem->mes = mes;
    }
    TAILQ_INSERT_HEAD(&head, elem, entries);
    printf("Inserted: %s\n", mes);
}

void send_to_log(char* module, char* message) {
    printf("Mes:%s\n", message);

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char time_buffer[30];
    char new_message[150];
    strftime(time_buffer, 25, "%x - %H:%M:%S", &tm);
    sprintf(new_message, "[%s]-|%s| %s", time_buffer, module, message);
    pthread_mutex_lock(&log_mutex);
    add_to_queue(new_message);
    pthread_cond_signal(&log_sent);
    pthread_mutex_unlock(&log_mutex);
}

void* logger_thread(void* arg) {
    // char ch = 'A';

    pthread_cond_t* log_signal = (pthread_cond_t*)arg;
    int i;
    struct entry* elem;
    FILE* file_ptr;

    TAILQ_INIT(&head);
    file_ptr = fopen("./log.txt", "a+");
    int charCount;
    charCount = ftell(file_ptr);
    fseek(file_ptr, charCount, SEEK_SET);
    fprintf(file_ptr, "DUUUUPA\n");

    while (flag_running) {
        // printf("HMM %d\n", TAILQ_EMPTY(&head));
        while (!(int)TAILQ_EMPTY(&head)) {
            printf("HMM %d\n", TAILQ_EMPTY(&head));
            elem = head.tqh_first;
            printf("Saving to log: %s\n", elem->mes);
            fprintf(file_ptr, "%s\n", elem->mes);
            fflush(file_ptr);
            TAILQ_REMOVE(&head, head.tqh_first, entries);
            printf("HMM2 %d\n", TAILQ_EMPTY(&head));
            // free(elem);
        }
        pthread_cond_wait(&log_sent, &log_mutex);
    }
    fclose(file_ptr);
}
