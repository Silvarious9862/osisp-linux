#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

extern volatile int terminate_flag;

int init_queue(MessageQueue *q, int capacity) {
    q->buffer = malloc(capacity * sizeof(Message));
    if (!q->buffer) return -1;
    q->capacity = capacity;
    q->count = 0;
    q->head = 0;
    q->tail = 0;
    q->added_count = 0;
    q->removed_count = 0;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->not_empty, NULL);
    pthread_cond_init(&q->not_full, NULL);
    return 0;
}

void destroy_queue(MessageQueue *q) {
    free(q->buffer);
    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->not_empty);
    pthread_cond_destroy(&q->not_full);
}

void push_message(MessageQueue *q, Message *m) {
    pthread_mutex_lock(&q->mutex);
    /* Ожидаем, пока очередь заполнена и система не завершает работу.
       Если terminate_flag установлен – выходим. */
    while (q->count == q->capacity && !terminate_flag) {
        pthread_cond_wait(&q->not_full, &q->mutex);
    }
    if (terminate_flag) {
        pthread_mutex_unlock(&q->mutex);
        return;
    }
    q->buffer[q->tail] = *m;
    q->tail = (q->tail + 1) % q->capacity;
    q->count++;
    q->added_count++;
    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->mutex);
}

void pop_message(MessageQueue *q, Message *m) {
    pthread_mutex_lock(&q->mutex);
    /* Ожидаем появления элемента в очереди, если очередь пуста и система не завершает работу */
    while (q->count == 0 && !terminate_flag) {
        pthread_cond_wait(&q->not_empty, &q->mutex);
    }
    if (terminate_flag) {
        pthread_mutex_unlock(&q->mutex);
        return;
    }
    *m = q->buffer[q->head];
    q->head = (q->head + 1) % q->capacity;
    q->count--;
    q->removed_count++;
    pthread_cond_signal(&q->not_full);
    pthread_mutex_unlock(&q->mutex);
}

int resize_queue(MessageQueue *q, int new_capacity) {
    pthread_mutex_lock(&q->mutex);
    if (new_capacity < q->count) {
        printf("Невозможно уменьшить очередь ниже количества элементов: %d\n", q->count);
        pthread_mutex_unlock(&q->mutex);
        return -1;
    }
    Message *new_buffer = malloc(new_capacity * sizeof(Message));
    if (!new_buffer) {
        pthread_mutex_unlock(&q->mutex);
        return -1;
    }
    /* Копируем элементы в порядке их логического расположения */
    for (int i = 0; i < q->count; i++) {
        new_buffer[i] = q->buffer[(q->head + i) % q->capacity];
    }
    free(q->buffer);
    q->buffer = new_buffer;
    q->capacity = new_capacity;
    q->head = 0;
    q->tail = q->count;
    /* Разбудим все ожидающие потоки */
    pthread_cond_broadcast(&q->not_full);
    pthread_cond_broadcast(&q->not_empty);
    pthread_mutex_unlock(&q->mutex);
    return 0;
}

void print_status(MessageQueue *q) {
    pthread_mutex_lock(&q->mutex);
    printf("\n--- Состояние очереди ---\n");
    printf("Ёмкость очереди: %d\n", q->capacity);
    printf("Элементов в очереди: %d\n", q->count);
    printf("Добавлено сообщений: %d\n", q->added_count);
    printf("Извлечено сообщений: %d\n", q->removed_count);
    pthread_mutex_unlock(&q->mutex);
}
