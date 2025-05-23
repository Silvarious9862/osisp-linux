#ifndef SORT_H
#define SORT_H

#include <stddef.h>
#include <pthread.h>
#include "index.h"  // Определение структуры struct index_s

/**
 * Структура sort_data хранит параметры для сортировки блоков:
 * - records: указатель на массив индексных записей (начало отображённой области, сразу после заголовка)
 * - block_size: число записей в одном блоке
 * - num_blocks: общее число блоков, на которые разбивается область (например, 2^granul)
 * - next_block: следующий номер свободного блока для сортировки; изначально равен числу потоков,
 *   поскольку первые блоки с номерами 0..(threads-1) сразу назначаются соответствующим потокам.
 * - mutex: мьютекс для защиты доступа к полю next_block
 * - barrier: барьер для синхронизации потоков по окончании сортировки всех блоков
 */
typedef struct {
    struct index_s *records;   // Массив записей, который нужно сортировать блоками
    size_t block_size;         // Число записей в одном блоке
    int num_blocks;            // Общее число блоков
    int next_block;            // Следующий свободный блок для сортировки
    pthread_mutex_t mutex;     // Мьютекс для защиты next_block
    pthread_barrier_t barrier; // Барьер для синхронизации завершения сортировки
} sort_data;

/**
 * Структура аргумента для потока сортировки.
 */
typedef struct {
    int thread_id;     // Идентификатор потока (0,1,...)
    sort_data *sd;     // Указатель на общую структуру параметров сортировки
} sort_thread_arg;

/**
 * Инициализирует структуру sort_data.
 *
 * @sd: указатель на структуру, которую нужно инициализировать
 * @records: указатель на массив записей (начало отображённой области после заголовка)
 * @total_records: общее число записей (можно использовать для дополнительной проверки)
 * @block_size: число записей в одном блоке (total_records должна делиться на число блоков)
 * @num_blocks: общее число блоков (например, 2^(granul))
 * @num_threads: число потоков, которые будут выполнять сортировку
 *
 * Возвращает 0 при успехе, -1 при ошибке.
 */
int sort_init(sort_data *sd, struct index_s *records, size_t total_records, size_t block_size, int num_blocks, int num_threads);

/**
 * Функция, выполняемая потоком. Каждый поток сначала сортирует блок с номером, равным его идентификатору,
 * а затем, с помощью конкурентного доступа через мьютекс, получает следующие свободные блоки для сортировки.
 * Когда свободных блоков не остаётся, поток синхронизируется с другими через барьер.
 *
 * @arg: указатель на структуру sort_thread_arg.
 *
 * Возвращает NULL при завершении.
 */
void *sort_blocks(void *arg);

#endif // SORT_H
