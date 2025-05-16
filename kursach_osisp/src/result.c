#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <limits.h>
#include "result.h"

// Объявление внешних глобальных переменных

extern size_t file_count;
extern struct file_entry *file_list;

/* Функция вывода подробного списка дубликатов.
   Группы файлов формируются по одинаковому размеру */
void print_filtered_file_list(void) {
    if (file_count == 0) {
        printf("Файлы-дубликаты не найдены\n");
        return;
    }

    size_t i = 0;
    while (i < file_count) {
        off_t current_size = file_list[i].file_size;
        size_t group_start = i;
        while (i < file_count && file_list[i].file_size == current_size) {
            i++;
        }
        for (size_t j = group_start; j < i; j++) {
            if (time_flag) {
                struct stat sb;
                if (stat(file_list[j].full_path, &sb) == 0) {
                    char timebuf[64];
                    struct tm *tm_info = localtime(&sb.st_mtime);
                    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", tm_info);
                    printf("%s\t%s\n", timebuf, file_list[j].full_path);
                } else {
                    printf("ERROR_TIME\t%s\n", file_list[j].full_path);
                }
            } else {
                printf("%s\n", file_list[j].full_path);
            }
        }
        printf("\n");
    }
}

/* Функция вывода сводной информации о найденных дубликатах.
   Подсчитывает количество групп, число файлов-дубликатов (которые можно удалить) и общий объем */
void print_summary(void) {
    if (file_count == 0) {
        printf("Файлы-дубликаты не найдены\n");
        return;
    }

    int duplicate_sets = 0;   /* число групп дубликатов */
    int duplicate_files = 0;  /* число файлов, которые можно удалить (из группы оставляем оригинал) */
    off_t total_waste = 0;    /* суммарный размер дубликатов */

    size_t i = 0;
    while (i < file_count) {
        off_t logical_size = file_list[i].file_size;
        int group_size = 0;
        off_t group_total = 0;
        off_t max_allocated = 0;

        while (i < file_count && file_list[i].file_size == logical_size) {
            struct stat sb;
            off_t allocated = 0;
            if(stat(file_list[i].full_path, &sb) == 0) {
                allocated = sb.st_blocks * 512;
            } else {
                allocated = logical_size;
            }
            group_total += allocated;
            if (allocated > max_allocated) max_allocated = allocated;
            group_size++;
            i++;
        }
        if (group_size > 1) {
            duplicate_sets++;
            duplicate_files += (group_size - 1);
            total_waste += (group_total - max_allocated);
        }
    }

    double wasteMB = total_waste / (1024.0 * 1024.0);
    printf("%d дублирующихся файлов (в %d наборах), занимают %.1f мегабайт\n",
           duplicate_files, duplicate_sets, wasteMB);
}

void print_size_listing(void) {
    if (file_count == 0) {
        printf("Файлы-дубликаты не найдены\n");
        return;
    }
    
    size_t i = 0;
    while (i < file_count)
    {
        off_t current_size = file_list[i].file_size;
        printf("%lld байт в каждом:\n", (long long) current_size);

        size_t group_start = i;
        while (i < file_count && file_list[i].file_size == current_size) {
            printf("%s\n", file_list[i].full_path);
            i++;
        }
        printf("\n");
    }
    
}

void print_size_time_listing(void) {
    if (file_count == 0) {
        printf("Файлы-дубликаты не найдены\n");
        return;
    }

    size_t i = 0;
    while (i < file_count) {
        off_t current_size = file_list[i].file_size;
        printf("%lld байт в каждом:\n", (long long)current_size);

        size_t group_start = i;
        /* Обрабатываем группу файлов, у которых одинаковый логический размер */
        while (i < file_count && file_list[i].file_size == current_size) {
            struct stat sb;
            if (stat(file_list[i].full_path, &sb) == 0) {
                char timebuf[64];
                struct tm *tm_info = localtime(&sb.st_mtime);
                strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M", tm_info);
                printf("%s %s\n", timebuf, file_list[i].full_path);
            } else {
                printf("ERROR_TIME %s\n", file_list[i].full_path);
            }
            i++;
        }
        printf("\n");
    }
}
