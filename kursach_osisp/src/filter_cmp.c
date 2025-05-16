#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "verbose.h"

/* Определение структуры для хранения информации о файле */
typedef struct {
    char full_path[PATH_MAX];
    off_t file_size;
} file_entry;

/* Глобальные переменные */
extern file_entry *file_list;
extern size_t file_count;
extern size_t file_list_capacity;

/* Функция для поблочного сравнения двух файлов с использованием memcmp().
   Открывает файлы, читает их блоками и сравнивает посредством memcmp().
   Возвращает 1, если файлы идентичны, и 0 в противном случае. */
static int files_are_identical(const char *path1, const char *path2) {
    FILE *fp1 = fopen(path1, "rb");
    if (!fp1) {
        perror("ошибка открытия первого файла");
        return 0;
    }
    FILE *fp2 = fopen(path2, "rb");
    if (!fp2) {
        perror("ошибка открытия второго файла");
        fclose(fp1);
        return 0;
    }
    
    char buf1[4096];
    char buf2[4096];
    size_t n1, n2;
    int identical = 1;
    while (1) {
        n1 = fread(buf1, 1, sizeof(buf1), fp1);
        n2 = fread(buf2, 1, sizeof(buf2), fp2);
        if(n1 != n2) {
            identical = 0;
            break;
        }
        if(n1 == 0) { // конец файлов
            break;
        }
        if(memcmp(buf1, buf2, n1) != 0) {
            identical = 0;
            break;
        }
    }
    fclose(fp1);
    fclose(fp2);
    return identical;
}

/* Функция filter_cmp_list:
   Проводит попарное сравнение файлов из глобального списка.
   Для каждой пары файлов с одинаковым размером, если файлы идентичны побайтно, оба файла маркируются как корректные.
   В дальнейшем, в новый список попадают только файлы, которые имеют хотя бы одного идентичного партнёра.
   Итоговый список сохраняется в глобальной переменной file_list, а file_count обновляется. */
void filter_cmp_list(void) {
    if (file_count == 0)
        return;

    /* Создаём временный массив-флаг для отметки файлов, которые имеют дубль */
    int *keep_flags = calloc(file_count, sizeof(int));
    if (!keep_flags) {
        perror("ошибка выделения памяти для keep_flags");
        exit(EXIT_FAILURE);
    }

    /* Сравниваем каждый файл с последующими */
    for (size_t i = 0; i < file_count; i++) {
        for (size_t j = i + 1; j < file_count; j++) {
            if (file_list[i].file_size == file_list[j].file_size) {
                if (files_are_identical(file_list[i].full_path, file_list[j].full_path)) {
                    keep_flags[i] = 1;
                    keep_flags[j] = 1;
                }
            }
        }
    }

    /* Собираем новый список, оставляя только файлы с совпадениями */
    file_entry *filtered_list = NULL;
    size_t filtered_count = 0;
    size_t filtered_capacity = 0;
    int unique_flag = 0;
    verbose_log("Уникальные файлы:");

    for (size_t i = 0; i < file_count; i++) {
        if (keep_flags[i]) {
            if (filtered_count >= filtered_capacity) {
                filtered_capacity = filtered_capacity ? filtered_capacity * 2 : 10;
                filtered_list = realloc(filtered_list, filtered_capacity * sizeof(file_entry));
                if (!filtered_list) {
                    perror("ошибка выделения памяти для filtered_list");
                    free(keep_flags);
                    exit(EXIT_FAILURE);
                }
            }
            filtered_list[filtered_count++] = file_list[i];
        } else {
            verbose_log_path(file_list[i].full_path);
            unique_flag = 1;
        }
    }
    if (!unique_flag) verbose_log("не найдены");

    free(keep_flags);
    free(file_list);
    file_list = filtered_list;
    file_count = filtered_count;
}
