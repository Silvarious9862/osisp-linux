#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "common.h"
#include "queue.h"
#include "producer_consumer.h"

// Глобальные переменные, объявленные как extern
extern MessageQueue queue;
extern volatile int terminate_flag;

unsigned short calculate_hash(Message *m) {
    unsigned short hash = 0;
    int len = (m->size == 0 ? DATA_SIZE : m->size);
    for (int i = 0; i < len; i++) {
        hash += (unsigned char)m->data[i];
    }
    return hash;
}

void *producer_thread(void *arg) {
    int id = *(int*)arg;
    free(arg);
    srand(time(NULL) ^ id);
    while (!terminate_flag) {
        Message m;
        // Генерация типа сообщения
        m.type = 'A' + (rand() % 26);
        int r = rand() % 257;
        while (r == 0)
            r = rand() % 257;
        int actual_len = (r == 256 ? 256 : r);
        m.size = (r == 256 ? 0 : r);
        int padded_len = ((actual_len + 3) / 4) * 4;
        if (padded_len > DATA_SIZE)
            padded_len = DATA_SIZE;
        for (int i = 0; i < padded_len; i++) {
            if (i < actual_len)
                m.data[i] = 'A' + (rand() % 26);
            else
                m.data[i] = 0;
        }
        m.hash = calculate_hash(&m);
        push_message(&queue, &m);
        printf("Producer[%d]: добавлено сообщение (тип '%c', размер %d, hash %u). Всего добавлено: %d\n",
               id, m.type, (m.size == 0 ? DATA_SIZE : m.size), m.hash, queue.added_count);
        sleep(3);
    }
    printf("Producer[%d] завершает работу\n", id);
    return NULL;
}
