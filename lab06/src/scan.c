#include "scan.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

/**
 * scan_file_segment() – отображает в память часть файла с учетом выравнивания.
 *
 * Сначала открывается файл с именем filename, затем вызывается mmap() для отображения
 * области размера map_length, начиная с map_offset. Если is_first == 1, то ожидается, что
 * по адресу (char *)map_ptr + adjustment находится 8-байтовый заголовок, а данные (index_s)
 * начинаются сразу после заголовка.
 */
scan_data *scan_file_segment(const char *filename, size_t map_length, off_t map_offset, off_t adjustment, int is_first, int num_blocks) {
    scan_data *data = malloc(sizeof(scan_data));
    if (!data) {
        perror("malloc scan_data");
        return NULL;
    }
    data->memsize = map_length;

    /* Открываем файл для чтения/записи */
    data->fd = open(filename, O_RDWR);
    if (data->fd < 0) {
        fprintf(stderr, "Error opening file %s: %s\n", filename, strerror(errno));
        free(data);
        return NULL;
    }

    /* Отображаем область файла начиная с map_offset */
    data->map_ptr = mmap(NULL, map_length, PROT_READ | PROT_WRITE, MAP_SHARED, data->fd, map_offset);
    if (data->map_ptr == MAP_FAILED) {
        fprintf(stderr, "Error mapping file segment at offset %ld: %s\n", (long)map_offset, strerror(errno));
        close(data->fd);
        free(data);
        return NULL;
    }

    if (is_first) {
        /* Извлекаем заголовок (общий число записей) по адресу: map_ptr + adjustment */
        data->total_records = *((uint64_t *)((char *)data->map_ptr + adjustment));
        /* Данные начинаются после заголовка */
        data->records = (struct index_s *)((char *)data->map_ptr + adjustment + sizeof(uint64_t));
    } else {
        data->total_records = 0; /* Для остальных сегментов заголовок не используется */
        data->records = (struct index_s *)((char *)data->map_ptr + adjustment);
    }

    return data;
}

void free_scan_data(scan_data *data) {
    if (data) {
        if (data->map_ptr && data->map_ptr != MAP_FAILED) {
            munmap(data->map_ptr, data->memsize);
        }
        if (data->fd >= 0) {
            close(data->fd);
        }
        free(data);
    }
}
