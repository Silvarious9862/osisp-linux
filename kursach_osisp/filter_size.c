#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <time.h>
#include "result.h"

/* Определение структуры file_entry такое же, как и в selection.c */
/*typedef struct
{
    char full_path[PATH_MAX];
    off_t file_size;
} file_entry;
*/
/* Объявляем глобальные переменные, определённые в selection.c, как extern */
extern file_entry *file_list;
//extern size_t file_count;
extern size_t file_list_capacity;
//extern int time_flag;

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
    if (file_count == 0)
        return;

    qsort(file_list, file_count, sizeof(file_entry), compare_file_entry);

    file_entry *filtered_list = NULL;
    size_t filtered_count = 0;
    size_t filtered_capacity = 0;

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
        }
        i = j;
    }

    free(file_list);
    file_list = filtered_list;
    file_count = filtered_count;
}

/* Вывод отфильтрованного списка файлов
void print_filtered_file_list(void)
{
    if (file_count == 0)
    {
        printf("Файлы-дубликаты не найдены\n");
        return;
    }

    printf("Найденные группы дубликатов:\n\n");

    size_t i = 0;
    while (i < file_count)
    {
        // Используем размер файла как ключ для группировки
        off_t current_size = file_list[i].file_size;
        // printf("Группа (размер: %lld байт):\n", (long long)current_size);

        size_t group_start = i;
        while (i < file_count && file_list[i].file_size == current_size)
        {
            i++;
        }

        for (size_t j = group_start; j < i; j++)
        {
            if (time_flag)
            {
                struct stat sb;
                if (stat(file_list[j].full_path, &sb) == 0)
                {
                    char timebuf[64];
                    struct tm *tm_info = localtime(&sb.st_mtime);
                    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", tm_info);
                    printf("%s\t%s\n", timebuf, file_list[j].full_path);
                }
                else
                {
                    printf("ERROR_TIME\t%s\n", file_list[j].full_path);
                }
            }
            else
            {
                printf("%s\n", file_list[j].full_path);
            }
        }
        // Разделитель между группами
        printf("\n");
    }
}
*/