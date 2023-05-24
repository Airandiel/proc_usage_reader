#include <pthread.h>
#include <sys/queue.h>

struct entry {
    char* mes;
    TAILQ_ENTRY(entry) entries;
};
TAILQ_HEAD(tailhead, entry) head;
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t log_sent = PTHREAD_COND_INITIALIZER;

void add_to_queue(char* mes);
void send_to_log(char* module, char* message);
void* logger_thread(void* arg);
