#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <magic.h>
#include "verbose.h"

// Определение структуры для хранения информации о файле.
typedef struct {
    char full_path[PATH_MAX];
    off_t file_size;
} file_entry;

/* Глобальные переменные */
extern file_entry *file_list;
extern size_t file_count;
extern size_t file_list_capacity;

/* Прототипы функций данного модуля */
void filter_mime_list(void);
void print_filtered_file_list(void);

/* Функция filter_mime_list:
   Для каждого набора файлов с одинаковым размером
   получает MIME-тип с помощью libmagic и в рамках группы оставляет только те файлы,
   MIME-тип которых встречается более одного раза. */
void filter_mime_list(void) {
    if (file_count == 0)
        return;

    /* Инициализация libmagic */
    magic_t cookie = magic_open(MAGIC_MIME);
    if (cookie == NULL) {
        fprintf(stderr, "не удалось инициализировать libmagic\n");
        exit(EXIT_FAILURE);
    }
    if (magic_load(cookie, NULL) != 0) {
        fprintf(stderr, "не удалось загрузить базу libmagic: %s\n", magic_error(cookie));
        magic_close(cookie);
        exit(EXIT_FAILURE);
    }

    file_entry *filtered_list = NULL;
    size_t filtered_count = 0;
    size_t filtered_capacity = 0;
    int unique_flag = 0;
    verbose_log("Уникальные файлы:");

    size_t i = 0;
    while (i < file_count) {
        /* Находим группу файлов с одинаковым размером */
        size_t j = i + 1;
        while (j < file_count && file_list[j].file_size == file_list[i].file_size) {
            j++;
        }
        size_t group_size = j - i;

        /* Выделяем временный массив для хранения MIME-типов группы. */
        char **mime_array = malloc(group_size * sizeof(char *));
        if (!mime_array) {
            perror("ошибка выделения памяти");
            magic_close(cookie);
            exit(EXIT_FAILURE);
        }

        /* Получаем MIME-тип для каждого файла в группе.
           Результат magic_file сразу копируем через strdup, чтобы строка сохранялась при последующих вызовах. */
        for (size_t k = 0; k < group_size; k++) {
            const char *mime = magic_file(cookie, file_list[i + k].full_path);
            if (mime) {
                mime_array[k] = strdup(mime);
                if (!mime_array[k]) {
                    perror("ошибка выделения памяти для mime");
                    for (size_t m = 0; m < k; m++) {
                        free(mime_array[m]);
                    }
                    free(mime_array);
                    magic_close(cookie);
                    exit(EXIT_FAILURE);
                }
            } else {
                mime_array[k] = NULL;
            }
        }

        /* Для каждого файла в группе вычисляем, сколько файлов имеют тот же MIME-тип */
        for (size_t k = 0; k < group_size; k++) {
            if (mime_array[k] == NULL)
                continue;
            int count = 0;
            for (size_t l = 0; l < group_size; l++) {
                if (mime_array[l] && strcmp(mime_array[k], mime_array[l]) == 0) {
                    count++;
                }
            }
            /* Если MIME-тип встречается более одного раза, добавляем файл в результирующий фильтр */
            if (count > 1) {
                if (filtered_count >= filtered_capacity) {
                    filtered_capacity = filtered_capacity ? filtered_capacity * 2 : 10;
                    filtered_list = realloc(filtered_list, filtered_capacity * sizeof(file_entry));
                    if (!filtered_list) {
                        perror("ошибка выделения памяти");
                        for (size_t m = 0; m < group_size; m++) {
                            free(mime_array[m]);
                        }
                        free(mime_array);
                        magic_close(cookie);
                        exit(EXIT_FAILURE);
                    }
                }
                filtered_list[filtered_count++] = file_list[i + k];
            } else {
                verbose_log_path(file_list[i].full_path);
                unique_flag = 1;
            }
        }

        /* Очистка временного массива с MIME-типами */
        for (size_t k = 0; k < group_size; k++) {
            free(mime_array[k]);
        }
        free(mime_array);

        i = j;
    }
    if (!unique_flag) verbose_log("не найдены");

    free(file_list);
    file_list = filtered_list;
    file_count = filtered_count;

    magic_close(cookie);
}