#ifndef SCAN_H
#define SCAN_H

#include <stdint.h>
#include <stddef.h>
#include "index.h"  // Определение struct index_s
#include <sys/stat.h>

/**
 * Структура scan_data хранит информацию об отображении файла:
 * - fd: дескриптор файла,
 * - map_ptr: адрес отображённой области,
 * - memsize: размер отображённой области,
 * - total_records: значение, считанное из заголовка (количество записей),
 * - records: указатель на массив записей (начало после заголовка).
 */
typedef struct {
    int fd;                 // Дескриптор файла
    void *map_ptr;          // Адрес отображённой области
    size_t memsize;         // Размер отображённой области
    uint64_t total_records; // Количество записей (из заголовка)
    struct index_s *records; // Указатель на массив записей (начало после заголовка)
} scan_data;

/**
 * scan_file() – функция отображения файла в память.
 *
 * @filename: имя файла, который нужно отобразить;
 * @memsize: размер области, которая отображается в память (должен быть кратен размеру страницы).
 *
 * Функция открывает файл, отображает его с помощью mmap() и считывает первые 8 байт
 * (заголовок, содержащий количество записей). Возвращается указатель на структуру scan_data
 * или NULL в случае ошибки.
 */
scan_data *scan_file(const char *filename, size_t memsize);

scan_data *scan_file_segment(const char *filename, size_t map_length, off_t map_offset, off_t adjustment, int is_first, int num_blocks);

/**
 * free_scan_data() – освобождает ресурсы (отменяет отображение и закрывает файл).
 *
 * @data: указатель на структуру scan_data, возвращённую функцией scan_file().
 */
void free_scan_data(scan_data *data);

#endif // SCAN_H
