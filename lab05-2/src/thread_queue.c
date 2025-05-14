#include "thread_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

// Глобальный mьютекс для защиты операций изменения размера очереди.
// Он должен быть определён в основном модуле (например, main.c),
// поэтому здесь мы объявляем его как external.
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
    // Инициализируем семафор свободных слотов значением capacity (так как все слоты свободны)
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
    pthread_mutex_lock(&resize_mutex);
    pthread_mutex_lock(&q->mutex);

    if (new_capacity < q->count) {
        printf("Невозможно уменьшить очередь ниже количества элементов: %d\n", q->count);
        pthread_mutex_unlock(&q->mutex);
        pthread_mutex_unlock(&resize_mutex);
        return -1;
    }

    int old_capacity = q->capacity;
    // Попытаемся перераспределить память для нового размера
    Message *new_buffer = realloc(q->buffer, new_capacity * sizeof(Message));
    if (!new_buffer) {
        pthread_mutex_unlock(&q->mutex);
        pthread_mutex_unlock(&resize_mutex);
        return -1;
    }
    q->buffer = new_buffer;
    q->capacity = new_capacity;

    // Если кольцевой буфер не начинается с нуля, переместим элементы так, чтобы head стал 0.
    if (q->head != 0 && q->count > 0) {
        Message *temp = malloc(q->capacity * sizeof(Message));
        if (temp) {
            for (int i = 0; i < q->count; i++) {
                temp[i] = q->buffer[(q->head + i) % old_capacity];
            }
            q->head = 0;
            q->tail = q->count;
            for (int i = 0; i < q->count; i++) {
                q->buffer[i] = temp[i];
            }
            free(temp);
        }
    }

    // Корректируем семафор свободных слотов:
    // Старое количество свободных мест = old_capacity - q->count.
    // Новое количество свободных мест = new_capacity - q->count.
    int free_old = old_capacity - q->count;
    int free_new = new_capacity - q->count;
    int delta = free_new - free_old;  // если delta > 0, увеличиваем значение семафора; если delta < 0 – уменьшаем.

    if (delta > 0) {
        for (int i = 0; i < delta; i++) {
            sem_post(&q->sem_free);
        }
    } else if (delta < 0) {
        for (int i = 0; i < -delta; i++) {
            // sem_trywait() уменьшает значение семафора, если свободное место имеется.
            sem_trywait(&q->sem_free);
        }
    }

    printf("Resize: old_capacity=%d, new_capacity=%d, count=%d, delta free=%d\n", 
           old_capacity, new_capacity, q->count, delta);
    printf("Размер очереди изменен: новая ёмкость = %d (занято %d)\n", q->capacity, q->count);

    pthread_mutex_unlock(&q->mutex);
    pthread_mutex_unlock(&resize_mutex);
    return 0;
}
