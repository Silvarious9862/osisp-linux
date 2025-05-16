// consumer.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include "common.h"
#include "thread_queue.h"
#include "producer_consumer.h"

extern volatile int terminate_flag;
extern ThreadMessageQueue queue;
extern pthread_mutex_t resize_mutex;

// Если calculate_hash уже вынесен в producer.c, можно вынести его общий прототип в producer_consumer.h.
unsigned short calculate_hash(Message *message);

void *consumer_thread(void *arg) {
    int id = *(int*)arg;
    free(arg);
    while (!terminate_flag) {
        pthread_mutex_lock(&resize_mutex);
        pthread_mutex_unlock(&resize_mutex);
        
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 1;
        while (sem_timedwait(&queue.sem_full, &ts) == -1) {
            if (errno == ETIMEDOUT) {
                if (terminate_flag) return NULL;
                clock_gettime(CLOCK_REALTIME, &ts);
                ts.tv_sec += 1;
            }
        }
        
        pthread_mutex_lock(&queue.mutex);
            Message message = queue.buffer[queue.head];
            queue.head = (queue.head + 1) % queue.capacity;
            queue.count--;
            queue.removed_count++;
        pthread_mutex_unlock(&queue.mutex);
        sem_post(&queue.sem_free);

        printf("Consumer[%d]: извлечено сообщение (тип '%c', размер %d, hash %u). Всего извлечено: %d\n",
                id, message.type, (message.size == 0 ? 256 : message.size), message.hash, queue.removed_count);
        if (calculate_hash(&message) == message.hash)
            printf("Consumer[%d]: сообщение корректно.\n", id);
        else
            printf("Consumer[%d]: сообщение повреждено!\n", id);

        sleep(3);
    }
    printf("Consumer[%d] завершает работу\n", id);
    return NULL;
}
