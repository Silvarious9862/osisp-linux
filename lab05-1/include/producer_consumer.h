#ifndef PRODUCER_CONSUMER_H
#define PRODUCER_CONSUMER_H

#include "common.h"
#include "thread_queue.h"
#include <pthread.h>

// Прототипы функций потока-производителя и потока-потребителя
void *producer_thread(void *arg);
void *consumer_thread(void *arg);

// Функция вычисления контрольной суммы сообщения
unsigned short calculate_hash(Message *message);

#endif // PRODUCER_CONSUMER_H
