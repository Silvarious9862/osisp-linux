#ifndef PREP_H
#define PREP_H

#include <pthread.h>

// Структура для передачи параметров потоку.
typedef struct thread_arg {
    int thread_id;    // Уникальный идентификатор потока (0, 1, ..., num_threads-1)
    int num_threads;  // Общее число потоков
    // При необходимости можно добавить дополнительные поля (например, указатель на буфер, размер блока и т.д.)
} thread_arg;

// Глобальный объект барьера для синхронизации потоков.
// Объявляется как extern, определение должно быть в prep.c.
extern pthread_barrier_t prep_barrier;
extern int prep_barrier_initialized;

/**
 * prep_init_barrier() - Инициализирует барьер для синхронизации потоков.
 * @num_threads: число потоков, с которым будет работать барьер.
 *
 * Возвращает 0 при успехе, -1 в случае ошибки.
 */
int prep_init_barrier(int num_threads);

/**
 * prep_create_threads() - Создаёт заданное число потоков.
 * @num_threads: общее число потоков для создания.
 * @thread_func: указатель на функцию, которую будут выполнять потоки.
 * @threads: адрес указателя, по которому будет выделен массив идентификаторов потоков.
 * @targs: адрес указателя, по которому будет выделен массив структур с аргументами для потоков.
 *
 * Функция выделяет память для массива потоков и аргументов. Для каждого потока задаётся
 * уникальный идентификатор и общее число потоков. При ошибке возвращает -1.
 */
int prep_create_threads(int num_threads,
                        void *(*thread_func)(void *),
                        pthread_t **threads,
                        thread_arg **targs);

/**
 * prep_join_threads() - Ожидает завершения работы всех потоков.
 * @num_threads: число потоков, которые необходимо объединить.
 * @threads: массив идентификаторов потоков.
 */
void prep_join_threads(int num_threads, pthread_t *threads);

#endif // PREP_H
