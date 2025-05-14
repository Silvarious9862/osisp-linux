#ifndef COMMON_H
#define COMMON_H

#define INITIAL_QUEUE_SIZE 10
#define MAX_QUEUE_SIZE 256   // максимальное число сообщений, которое может храниться
#define SHM_KEY 0x1234
#define SEM_KEY 0x5678

// Структура сообщения
typedef struct {
    char type;                 // тип сообщения (1 байт)
    unsigned short hash;       // контрольная сумма (2 байта)
    unsigned char size;        // длина данных (0 означает 256 байт)
    char data[256];            // данные сообщения (выравнены до кратного 4)
} Message;

// Структура очереди. Обратите внимание, что теперь поле buffer имеет размер MAX_QUEUE_SIZE,
// а текущая ёмкость хранится в поле capacity.
typedef struct {
    int capacity;              // текущая ёмкость очереди (<= MAX_QUEUE_SIZE)
    Message buffer[MAX_QUEUE_SIZE]; // буфер для сообщений
    int head;                  // индекс для извлечения (голова)
    int tail;                  // индекс для добавления (хвост)
    int added_count;           // количество добавленных сообщений
    int removed_count;         // количество извлечённых сообщений
    int free_slots;            // количество свободных мест (равно capacity - count)
} MessageQueue;

#endif // COMMON_H
