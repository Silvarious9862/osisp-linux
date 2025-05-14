#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "common.h"
#include "queue.h"
#include "producer_consumer.h"

// Глобальные переменные, объявленные как extern
extern MessageQueue queue;
extern volatile int terminate_flag;

// Используем прототип calculate_hash() из producer_consumer.h

void *consumer_thread(void *arg) {
    int id = *(int*)arg;
    free(arg);
    while (!terminate_flag) {
        Message m;
        pop_message(&queue, &m);
        printf("Consumer[%d]: извлечено сообщение (тип '%c', размер %d, hash %u). Всего извлечено: %d\n",
               id, m.type, (m.size == 0 ? DATA_SIZE : m.size), m.hash, queue.removed_count);
        if (calculate_hash(&m) == m.hash)
            printf("Consumer[%d]: сообщение корректно.\n", id);
        else
            printf("Consumer[%d]: сообщение повреждено!\n", id);
        sleep(3);
    }
    printf("Consumer[%d] завершает работу\n", id);
    return NULL;
}
