#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>
#include "common.h"

// Структура динамической (кольцевой) очереди
typedef struct {
    Message *buffer;          // Динамический массив сообщений
    int capacity;             // Текущая ёмкость очереди
    int count;                // Число сообщений в очереди
    int head;                 // Индекс для извлечения
    int tail;                 // Индекс для вставки
    int added_count;          // Счётчик добавленных сообщений (для отладки)
    int removed_count;        // Счётчик извлечённых сообщений (для отладки)
    pthread_mutex_t mutex;    // Мьютекс для защиты очереди
    pthread_cond_t not_empty; // Условная переменная: очередь не пуста
    pthread_cond_t not_full;  // Условная переменная: очередь имеет свободное место
} MessageQueue;

// Прототипы функций работы с очередью
int init_queue(MessageQueue *q, int capacity);
void destroy_queue(MessageQueue *q);
void push_message(MessageQueue *q, Message *m);
void pop_message(MessageQueue *q, Message *m);
int resize_queue(MessageQueue *q, int new_capacity);
void print_status(MessageQueue *q);

#endif // QUEUE_H