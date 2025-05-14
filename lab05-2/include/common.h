#ifndef COMMON_H
#define COMMON_H

// Размер буфера для данных сообщения
#define DATA_SIZE 256

// Начальная ёмкость очереди
#define INITIAL_CAPACITY 10

// Структура сообщения
typedef struct {
    char type;                 // Тип сообщения (один символ)
    unsigned short hash;       // Контрольная сумма сообщения
    unsigned char size;        // Если 0, то означает 256 байт
    char data[DATA_SIZE];      // Данные сообщения
} Message;

#endif // COMMON_H
