#ifndef COMMON_H
#define COMMON_H

#include <sys/ipc.h>

#define QUEUE_SIZE 10
#define SHM_KEY 0x1234
#define SEM_KEY 0x5678

// Определение структуры сообщения
typedef struct {
    char type;                 // Тип сообщения
    unsigned short hash;       // Контрольная сумма
    unsigned char size;        // Размер данных
    char data[256];            // Данные сообщения
} Message;

// Определение структуры очереди на основе кольцевого буфера
typedef struct {
    Message buffer[QUEUE_SIZE]; // Кольцевой буфер
    int head;                   // Индекс извлечения (голова)
    int tail;                   // Индекс добавления (хвост)
    int added_count;            // Количество добавленных сообщений
    int removed_count;          // Количество извлечённых сообщений
    int free_slots;             // Количество свободных слотов в буфере
} MessageQueue;

#endif // COMMON_H
