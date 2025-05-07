#define _POSIX_C_SOURCE 200809L // Активация POSIX API для C11
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

// Структура индексной записи
typedef struct {
    double time_mark; // временная метка
    uint64_t recno;   // номер записи
} index_s;

// Заголовок файла
typedef struct {
    uint64_t records;  // количество записей
    index_s idx[];     // массив записей
} index_hdr_s;

// Аргументы для потоков
typedef struct {
    index_s *start;
    size_t size;
    pthread_barrier_t *barrier;
    int thread_id;
} thread_arg;

// Функция сравнения для qsort
int compare_index(const void *a, const void *b) {
    const index_s *ia = (const index_s *)a;
    const index_s *ib = (const index_s *)b;
    return (ia->time_mark < ib->time_mark) ? -1 : (ia->time_mark > ib->time_mark);
}

// Функция сортировки блока
void *thread_sort(void *arg) {
    thread_arg *targ = (thread_arg *)arg;
    printf("Поток %d сортирует блок с адресом: %p, размер: %zu\n",
           targ->thread_id, targ->start, targ->size);

    qsort(targ->start, targ->size, sizeof(index_s), compare_index);

    printf("Поток %d завершил сортировку. Ожидание на барьере...\n", targ->thread_id);
    pthread_barrier_wait(targ->barrier);

    return NULL;
}

// Функция для слияния блоков
void merge_blocks(index_s *buffer, size_t total_records, size_t block_size) {
    index_s *temp = malloc(sizeof(index_s) * total_records);
    if (!temp) {
        perror("Ошибка выделения памяти для слияния");
        exit(EXIT_FAILURE);
    }

    size_t merged_size = block_size;
    while (merged_size < total_records) {
        for (size_t i = 0; i < total_records; i += 2 * merged_size) {
            size_t left = i;
            size_t right = left + merged_size;
            size_t end = (right + merged_size < total_records) ? (right + merged_size) : total_records;

            size_t l = left, r = right, t = left;
            while (l < right && r < end) {
                if (buffer[l].time_mark < buffer[r].time_mark) {
                    temp[t++] = buffer[l++];
                } else {
                    temp[t++] = buffer[r++];
                }
            }

            while (l < right) temp[t++] = buffer[l++];
            while (r < end) temp[t++] = buffer[r++];
        }
        memcpy(buffer, temp, total_records * sizeof(index_s));
        merged_size *= 2;
    }

    free(temp);
}

int main(int argc, char **argv) {
    if (argc < 5) {
        fprintf(stderr, "Usage: %s filename memsize threads blocks\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *filename = argv[1];
    size_t memsize = atoll(argv[2]);
    int threads = atoi(argv[3]);
    size_t blocks = atoi(argv[4]);

    if (memsize % blocks != 0) {
        fprintf(stderr, "Ошибка: memsize должен быть кратен blocks.\n");
        return EXIT_FAILURE;
    }

    size_t block_size = memsize / blocks;

    int fd = open(filename, O_RDWR);
    if (fd < 0) {
        perror("Ошибка открытия файла");
        return EXIT_FAILURE;
    }

    index_hdr_s *header = mmap(NULL, memsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (header == MAP_FAILED) {
        perror("Ошибка отображения памяти");
        close(fd);
        return EXIT_FAILURE;
    }

    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, threads + 1);

    pthread_t thread_pool[threads];
    thread_arg thread_args[threads];

    for (int i = 0; i < threads; i++) {
        thread_args[i].start = header->idx + i * (block_size / sizeof(index_s));
        thread_args[i].size = block_size / sizeof(index_s);
        thread_args[i].barrier = &barrier;
        thread_args[i].thread_id = i;
        pthread_create(&thread_pool[i], NULL, thread_sort, &thread_args[i]);
    }

    printf("Основной поток ожидает на барьере...\n");
    pthread_barrier_wait(&barrier);

    for (int i = 0; i < threads; i++) {
        pthread_join(thread_pool[i], NULL);
    }

    printf("Слияние блоков...\n");
    merge_blocks(header->idx, header->records, block_size / sizeof(index_s));

    printf("Проверка после сортировки:\n");
    for (size_t i = 0; i < header->records; i++) {
        printf("Запись %zu: временная метка: %lf\n", i, header->idx[i].time_mark);
    }

    msync(header, memsize, MS_SYNC);
    munmap(header, memsize);
    close(fd);

    printf("Сортировка завершена.\n");
    return EXIT_SUCCESS;
}
