#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include "result.h"
#include "selection.h"

/* Функция, объединяющая вывод размера группы (-S) и времени модификации (-t).
   Для каждой группы дубликатов выводится заголовок в виде:
     <file_size> bytes each:
   Затем для каждого файла в группе выводится время последней модификации и путь.
   Формат времени: "YYYY-MM-DD HH:MM" */
void print_size_time_listing(void) {
    if (file_count == 0) {
        printf("Файлы-дубликаты не найдены\n");
        return;
    }

    size_t i = 0;
    while (i < file_count) {
        off_t current_size = file_list[i].file_size;
        printf("%lld bytes each:\n", (long long)current_size);

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
