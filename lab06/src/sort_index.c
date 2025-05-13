// sort_index.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "scan.h"    // Доработанный модуль отображения файла с новой сигнатурой scan_file_segment()
#include "sort.h"    // Модуль сортировки блоков
#include "merge.h"   // Модуль слияния блоков
#include "write.h"   // Модуль записи результата в файл
#include "finish.h"  // Модуль очистки синхронизационных объектов

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s memsize granul threads filename\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* Чтение параметров запуска */
    long user_memsize = strtoul(argv[1], NULL, 10);
    int granul = atoi(argv[2]);
    int threads_count = atoi(argv[3]);
    char *filename = argv[4];

    if (user_memsize <= 0 || granul < 0 || threads_count <= 0) {
        fprintf(stderr, "Invalid parameters.\n");
        return EXIT_FAILURE;
    }
    int num_blocks = 1 << granul;
    if (num_blocks <= threads_count) {
        fprintf(stderr, "Error: number of blocks (%d) must be greater than threads (%d).\n", num_blocks, threads_count);
        return EXIT_FAILURE;
    }

    /* Получаем размер файла */
    struct stat st;
    if (stat(filename, &st) != 0) {
        perror("stat");
        return EXIT_FAILURE;
    }
    off_t filesize = st.st_size;
    if (filesize == 0) {
        fprintf(stderr, "Error: file is empty.\n");
        return EXIT_FAILURE;
    }

    /* Получаем размер страницы для выравнивания mmap */
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0) {
        perror("sysconf");
        return EXIT_FAILURE;
    }

    /* logical_offset – смещение в файле (логическое, без выравнивания),
       которое будет обновляться по мере обработки сегментов.
       Для первого сегмента оно равно 0. */
    off_t logical_offset = 0;

    while (logical_offset < filesize) {
        size_t remaining = filesize - logical_offset;
        size_t effective_length;
        int is_first = (logical_offset == 0) ? 1 : 0;
        if (is_first)
            effective_length = user_memsize + sizeof(uint64_t);  // для первого сегмента учитываем заголовок
        else
            effective_length = user_memsize;
        if (remaining < effective_length)
            effective_length = remaining;  // для последнего сегмента

        /* Вычисляем выравнённое смещение и adjustment */
        off_t aligned_offset = (logical_offset / page_size) * page_size;
        off_t adjustment = logical_offset - aligned_offset;

        /* Длина отображаемой области для mmap = adjustment + effective_length */
        size_t map_length = adjustment + effective_length;
        if ((size_t)(filesize - aligned_offset) < map_length)
            map_length = filesize - aligned_offset;

        /* Минимальный размер логического сегмента: для первого – header + 1 запись, для остальных – 1 запись */
        size_t min_effective = is_first ? (sizeof(uint64_t) + sizeof(struct index_s)) : sizeof(struct index_s);
        if (effective_length < min_effective)
            break;

        /* Вызов функции отображения. Передаём выровненное смещение */
        scan_data *seg_data = scan_file_segment(filename, map_length, aligned_offset, adjustment, is_first, num_blocks);
        if (!seg_data) {
            fprintf(stderr, "Error: cannot map file segment at offset %ld.\n", (long)aligned_offset);
            return EXIT_FAILURE;
        }

        /* Вычисляем число записей в сегменте на основе логического effective_length */
        size_t records_in_seg;
        if (is_first)
            records_in_seg = (effective_length - sizeof(uint64_t)) / sizeof(struct index_s);
        else
            records_in_seg = effective_length / sizeof(struct index_s);

        /* Округляем число записей вниз до ближайшего числа, кратного num_blocks */
        records_in_seg = (records_in_seg / num_blocks) * num_blocks;
        if (records_in_seg == 0) {
            free_scan_data(seg_data);
            break;
        }
        size_t block_size = records_in_seg / num_blocks;

        /* --- Этап сортировки сегмента --- */
        /* Для сортировки данные начинаются по адресу:
           seg_data->map_ptr + adjustment + (если is_first то sizeof(uint64_t) иначе 0) */
        char *data_ptr = (char *)seg_data->map_ptr + adjustment;
        if (is_first)
            data_ptr += sizeof(uint64_t);

        sort_data sd;
        if (sort_init(&sd, (struct index_s *)data_ptr, records_in_seg, block_size, num_blocks, threads_count) != 0) {
            free_scan_data(seg_data);
            return EXIT_FAILURE;
        }
        pthread_t *sort_threads = malloc(threads_count * sizeof(pthread_t));
        sort_thread_arg *sort_targs = malloc(threads_count * sizeof(sort_thread_arg));
        if (!sort_threads || !sort_targs) {
            perror("malloc");
            free_scan_data(seg_data);
            return EXIT_FAILURE;
        }
        for (int i = 0; i < threads_count; i++) {
            sort_targs[i].thread_id = i;
            sort_targs[i].sd = &sd;
            int ret = pthread_create(&sort_threads[i], NULL, sort_blocks, &sort_targs[i]);
            if (ret != 0) {
                fprintf(stderr, "Error creating sort thread %d: %s\n", i, strerror(ret));
                free(sort_threads);
                free(sort_targs);
                free_scan_data(seg_data);
                return EXIT_FAILURE;
            }
        }
        for (int i = 0; i < threads_count; i++)
            pthread_join(sort_threads[i], NULL);
        free(sort_threads);
        free(sort_targs);

        /* --- Этап слияния сегмента --- */
        merge_data md;
        if (merge_init(&md, (struct index_s *)data_ptr, block_size, num_blocks, threads_count) != 0) {
            free_scan_data(seg_data);
            return EXIT_FAILURE;
        }
        pthread_t *merge_threads = malloc(threads_count * sizeof(pthread_t));
        merge_thread_arg *merge_targs = malloc(threads_count * sizeof(merge_thread_arg));
        if (!merge_threads || !merge_targs) {
            perror("malloc");
            free_scan_data(seg_data);
            return EXIT_FAILURE;
        }
        for (int i = 0; i < threads_count; i++) {
            merge_targs[i].thread_id = i;
            merge_targs[i].num_threads = threads_count;
            merge_targs[i].md = &md;
            int ret = pthread_create(&merge_threads[i], NULL, merge_blocks_phase, &merge_targs[i]);
            if (ret != 0) {
                fprintf(stderr, "Error creating merge thread %d: %s\n", i, strerror(ret));
                free(merge_threads);
                free(merge_targs);
                free_scan_data(seg_data);
                return EXIT_FAILURE;
            }
        }
        for (int i = 0; i < threads_count; i++)
            pthread_join(merge_threads[i], NULL);
        free(merge_threads);
        free(merge_targs);

        /* --- Этап записи отсортированного сегмента --- */
        if (write_sorted_result(seg_data) != 0) {
            free_scan_data(seg_data);
            return EXIT_FAILURE;
        }

        /* Очистка объектов, созданных для данного сегмента */
        finish_cleanup_merge(&md);
        finish_cleanup_sort(&sd);
        free_scan_data(seg_data);

        /* Обновляем logical_offset:
           для первого сегмента – на (user_memsize + header),
           для остальных – на user_memsize */
        if (is_first)
            logical_offset += (user_memsize + sizeof(uint64_t));
        else
            logical_offset += user_memsize;
    }

    /* Очистка глобального объекта, если используется (например, prep_barrier) */
//    finish_cleanup_prep();
    return EXIT_SUCCESS;
}
