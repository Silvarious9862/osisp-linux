// prep.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "prep.h"

// Определение глобального барьера, объявленного в prep.h
pthread_barrier_t prep_barrier;
int prep_barrier_initialized = 0;

/**
 * prep_init_barrier() - Инициализирует барьер для синхронизации потоков.
 * @num_threads: число потоков, с которым будет работать барьер.
 *
 * Возвращает 0 при успехе, -1 в случае ошибки.
 */
int prep_init_barrier(int num_threads) {
    int ret = pthread_barrier_init(&prep_barrier, NULL, num_threads);
    if (ret != 0) {
        fprintf(stderr, "Error initializing barrier: %s\n", strerror(ret));
        return -1;
    }

    prep_barrier_initialized = 1;
    return 0;
}

/**
 * prep_create_threads() - Создаёт заданное число потоков.
 * @num_threads: общее число потоков для создания.
 * @thread_func: указатель на функцию, которую будут выполнять потоки.
 * @threads: адрес указателя, по которому будет выделен массив идентификаторов потоков.
 * @targs: адрес указателя, по которому будет выделен массив структур с аргументами для потоков.
 *
 * Функция выделяет память для массива потоков и аргументов. Каждый поток получает свой
 * уникальный идентификатор и общее число потоков. При ошибке возвращает -1.
 */
int prep_create_threads(int num_threads,
                        void *(*thread_func)(void *),
                        pthread_t **threads,
                        thread_arg **targs)
{
    *threads = malloc(num_threads * sizeof(pthread_t));
    if (*threads == NULL) {
        perror("malloc threads");
        return -1;
    }

    *targs = malloc(num_threads * sizeof(thread_arg));
    if (*targs == NULL) {
        perror("malloc thread args");
        free(*threads);
        return -1;
    }

    for (int i = 0; i < num_threads; i++) {
        (*targs)[i].thread_id = i;
        (*targs)[i].num_threads = num_threads;
        int ret = pthread_create(&(*threads)[i], NULL, thread_func, &(*targs)[i]);
        if (ret != 0) {
            fprintf(stderr, "Error creating thread %d: %s\n", i, strerror(ret));
            free(*threads);
            free(*targs);
            return -1;
        }
    }
    return 0;
}

/**
 * prep_join_threads() - Ожидает завершения работы всех потоков.
 * @num_threads: число потоков, которые необходимо объединить.
 * @threads: массив идентификаторов потоков.
 */
void prep_join_threads(int num_threads, pthread_t *threads) {
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
}
