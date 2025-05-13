#include "thread_queue.h"
#include <stdio.h>
#include <stdlib.h>

int init_thread_queue(ThreadMessageQueue *q, int capacity) {
    q->buffer = malloc(capacity * sizeof(Message));
    if (!q->buffer) return -1;
    q->capacity = capacity;
    q->head = 0;
    q->tail = 0;
    q->count = 0;
    q->added_count = 0;
    q->removed_count = 0;
    pthread_mutex_init(&q->mutex, NULL);
    sem_init(&q->sem_free, 0, capacity);  // все слоты свободны
    sem_init(&q->sem_full, 0, 0);
    return 0;
}

void destroy_thread_queue(ThreadMessageQueue *q) {
    free(q->buffer);
    pthread_mutex_destroy(&q->mutex);
    sem_destroy(&q->sem_free);
    sem_destroy(&q->sem_full);
}

int resize_thread_queue(ThreadMessageQueue *q, int new_capacity) {
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
    // Перекопирование элементов в порядке очереди
    for (int i = 0; i < q->count; i++) {
        new_buffer[i] = q->buffer[(q->head + i) % q->capacity];
    }
    free(q->buffer);
    q->buffer = new_buffer;
    q->capacity = new_capacity;
    q->head = 0;
    q->tail = q->count;
    // Пересоздание семафоров
    sem_destroy(&q->sem_free);
    sem_destroy(&q->sem_full);
    sem_init(&q->sem_free, 0, new_capacity - q->count);
    sem_init(&q->sem_full, 0, q->count);
    pthread_mutex_unlock(&q->mutex);
    printf("Размер очереди изменен: новая ёмкость = %d\n", new_capacity);
    return 0;
}
