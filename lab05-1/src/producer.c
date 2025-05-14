// producer.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include "common.h"
#include "thread_queue.h"
#include "producer_consumer.h"  // Если в нём содержатся дополнительные прототипы

extern volatile int terminate_flag;   // Глобальный флаг завершения (определён в main.c)
extern ThreadMessageQueue queue;        // Глобальная очередь
extern pthread_mutex_t resize_mutex;      // Глобальный mutex для защиты при изменении размера
extern volatile int pause_processing;

// Функция вычисления контрольной суммы; можно её вынести в общий модуль, если используется и в consumer.c.
unsigned short calculate_hash(Message *message) {
    unsigned short hash = 0;
    int len = (message->size == 0 ? 256 : message->size);
    for (int i = 0; i < len; i++)
        hash += (unsigned char) message->data[i];
    return hash;
}

void *producer_thread(void *arg) {
    int id = *(int*)arg;
    free(arg);
    
    while (!terminate_flag) {
        Message message;
        message.type = 'A' + (rand() % 26);
        int r = rand() % 257;
        while (r == 0)
            r = rand() % 257;
        int actual_len = (r == 256) ? 256 : r;
        message.size = (r == 256) ? 0 : r;

        int padded_len = ((actual_len + 3) / 4) * 4;
        if (padded_len > (int)sizeof(message.data))
            padded_len = sizeof(message.data);
        for (int i = 0; i < padded_len; i++) {
            if (i < actual_len)
                message.data[i] = 'A' + (rand() % 26);
            else
                message.data[i] = 0;
        }
        message.hash = calculate_hash(&message);

        // Производитель ждет, если обработка приостановлена
        while (pause_processing) {
            pthread_yield();
        }

        // Защита перед ожиданием sem_wait через resize_mutex
        pthread_mutex_lock(&resize_mutex);
        pthread_mutex_unlock(&resize_mutex);

        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 1;
        while (sem_timedwait(&queue.sem_free, &ts) == -1) {
            if (errno == ETIMEDOUT) {
                if (terminate_flag) return NULL;
                clock_gettime(CLOCK_REALTIME, &ts);
                ts.tv_sec += 1;
            }
        }

        pthread_mutex_lock(&queue.mutex);
            queue.buffer[queue.tail] = message;
            queue.tail = (queue.tail + 1) % queue.capacity;
            queue.count++;
            queue.added_count++;
            printf("Producer[%d]: добавлено сообщение (тип '%c', размер %d, hash %u). Всего добавлено: %d\n",
                id, message.type, (message.size == 0 ? 256 : message.size), message.hash, queue.added_count);
        pthread_mutex_unlock(&queue.mutex);
        sem_post(&queue.sem_full);

        sleep(3);
    }
    printf("Producer[%d] завершает работу\n", id);
    return NULL;
}
