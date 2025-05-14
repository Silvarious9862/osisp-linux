#ifndef PRODUCER_CONSUMER_H
#define PRODUCER_CONSUMER_H

#include "common.h"

// Прототипы функций потоков
void *producer_thread(void *arg);
void *consumer_thread(void *arg);

// Функция вычисления контрольной суммы сообщения
unsigned short calculate_hash(Message *m);

#endif // PRODUCER_CONSUMER_H
