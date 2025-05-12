#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <time.h>
#include "result.h"
#include "verbose.h"

/* Объявляем глобальные переменные как extern */
extern file_entry *file_list;
extern size_t file_list_capacity;

/* Прототипы функций данного модуля */
int compare_file_entry(const void *a, const void *b);
void filter_file_list(void);
void print_filtered_file_list(void);

/* Функция сравнения для сортировки по размеру */
int compare_file_entry(const void *a, const void *b)
{
    const file_entry *fa = (const file_entry *)a;
    const file_entry *fb = (const file_entry *)b;
    if (fa->file_size < fb->file_size)
        return -1;
    if (fa->file_size > fb->file_size)
        return 1;
    return 0;
}

/* Фильтрация списка файлов: оставляем только файлы, у которых есть дубликаты (одинаковый размер) */
void filter_file_list(void)
{
    if (file_count == 0){
        return;
    }

    qsort(file_list, file_count, sizeof(file_entry), compare_file_entry);

    file_entry *filtered_list = NULL;
    size_t filtered_count = 0;
    size_t filtered_capacity = 0;
    int unique_flag = 0;
    verbose_log("Уникальные файлы:");

    for (size_t i = 0; i < file_count;)
    {
        size_t j = i + 1;
        /* Собираем группу файлов с одинаковым размером */
        while (j < file_count && file_list[j].file_size == file_list[i].file_size)
        {
            j++;
        }
        

        if ((j - i) > 1)
        { /* Если найдено более одного файла одного размера */
            size_t group_size = j - i;
            if (filtered_count + group_size > filtered_capacity)
            {
                filtered_capacity = filtered_capacity ? filtered_capacity * 2 : 10;
                filtered_list = realloc(filtered_list, filtered_capacity * sizeof(file_entry));
                if (!filtered_list)
                {
                    perror("ошибка выделения памяти");
                    exit(EXIT_FAILURE);
                }
            }
            for (size_t k = i; k < j; k++)
            {
                filtered_list[filtered_count++] = file_list[k];
            }
        } else {
            verbose_log_path(file_list[i].full_path);
            unique_flag = 1;
        }
        i = j;
    }
    if (!unique_flag) verbose_log("не найдены");

    free(file_list);
    file_list = filtered_list;
    file_count = filtered_count;
}