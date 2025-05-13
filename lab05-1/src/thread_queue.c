#include "thread_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Объявляется глобальный mutex, используемый при изменении размера очереди.
// Он должен быть определён в основном модуле, например, в main.c.
extern pthread_mutex_t resize_mutex;

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
    // Все слоты свободны изначально, поэтому семафор свободных мест = capacity,
    // а заполненных – 0.
    sem_init(&q->sem_free, 0, capacity);
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
    // Блокируем resize_mutex, чтобы никакие потоки не начали ждать семафоры
    // в момент изменения размера.
    pthread_mutex_lock(&resize_mutex);
    
    pthread_mutex_lock(&q->mutex);
    if (new_capacity < q->count) {
        printf("Невозможно уменьшить очередь ниже количества элементов: %d\n", q->count);
        pthread_mutex_unlock(&q->mutex);
        pthread_mutex_unlock(&resize_mutex);
        return -1;
    }
    
    Message *new_buffer = malloc(new_capacity * sizeof(Message));
    if (!new_buffer) {
        pthread_mutex_unlock(&q->mutex);
        pthread_mutex_unlock(&resize_mutex);
        return -1;
    }
    
    // Копируем элементы в правильном порядке:
    // элемент с индексом 0 нового буфера = q->buffer[q->head],
    // затем q->buffer[(q->head+1) % q->capacity] и т.д.
    for (int i = 0; i < q->count; i++) {
        new_buffer[i] = q->buffer[(q->head + i) % q->capacity];
    }
    
    free(q->buffer);
    q->buffer = new_buffer;
    q->capacity = new_capacity;
    q->head = 0;
    q->tail = q->count;  // т.к. элементы теперь располагаются с 0 до count-1

    // Пересоздаём семафоры.
    // Новое значение свободных мест = new_capacity - q->count.
    sem_destroy(&q->sem_free);
    sem_destroy(&q->sem_full);
    sem_init(&q->sem_free, 0, new_capacity - q->count);
    sem_init(&q->sem_full, 0, q->count);
    
    pthread_mutex_unlock(&q->mutex);
    printf("Размер очереди изменен: новая ёмкость = %d (занято %d)\n", new_capacity, q->count);
    pthread_mutex_unlock(&resize_mutex);
    return 0;
}
