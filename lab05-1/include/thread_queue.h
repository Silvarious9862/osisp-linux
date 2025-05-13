#ifndef THREAD_QUEUE_H
#define THREAD_QUEUE_H

#include "common.h"
#include <pthread.h>
#include <semaphore.h>

// Динамическая очередь сообщений для мультипоточности
typedef struct {
    Message *buffer;           // динамический массив сообщений
    int capacity;              // текущая ёмкость очереди
    int head;                  // индекс извлечения
    int tail;                  // индекс добавления
    int count;                 // число сообщений в очереди
    int added_count;           // общий счётчик добавленных сообщений
    int removed_count;         // общий счётчик извлечённых сообщений
    pthread_mutex_t mutex;     // мьютекс для защиты очереди
    sem_t sem_free;            // семафор для свободных слотов
    sem_t sem_full;            // семафор для заполненных слотов
} ThreadMessageQueue;

// Инициализация очереди; возвращает 0 при успехе, -1 при отказе.
int init_thread_queue(ThreadMessageQueue *q, int capacity);

// Освобождение очереди и освобождение ресурсов.
void destroy_thread_queue(ThreadMessageQueue *q);

// Изменение ёмкости очереди (new_capacity не может быть меньше числа элементов).
int resize_thread_queue(ThreadMessageQueue *q, int new_capacity);

#endif // THREAD_QUEUE_H
