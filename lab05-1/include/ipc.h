#ifndef IPC_H
#define IPC_H

#include "common.h"

// Простая функция-обёртка для работы с семафорами
void semaphore_op(int sem_id, int sem_num, int op);

// Функция для вычисления контрольной суммы сообщения
unsigned short calculate_hash(Message* message);

// Функция для проверки контрольной суммы сообщения
int verify_hash(Message* message);

#endif // IPC_H
