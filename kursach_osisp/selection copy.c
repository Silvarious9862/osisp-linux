#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>
#include "verbose.h"

extern int recursive_flag;

typedef struct {
    char full_path[PATH_MAX];
    off_t file_size;
} file_entry;

/* Глобальные переменные для хранения списка файлов */
file_entry *file_list = NULL;
size_t file_count = 0;
size_t file_list_capacity = 0;

/* Прототипы функций данного модуля */
void add_file_entry(const char *full_path, off_t file_size);
void scan_directory(const char *path);

/* Функция для добавления записи о файле в глобальный список */
void add_file_entry(const char *full_path, off_t file_size) {
    if (file_count >= file_list_capacity) {
        file_list_capacity = file_list_capacity ? file_list_capacity * 2 : 10;
        file_list = realloc(file_list, file_list_capacity * sizeof(file_entry));
        if (!file_list) {
            perror("ошибка выделения памяти");
            exit(EXIT_FAILURE);
        }
    }
    strncpy(file_list[file_count].full_path, full_path, PATH_MAX);
    file_list[file_count].full_path[PATH_MAX - 1] = '\0';
    file_list[file_count].file_size = file_size;
    file_count++;
}

/* Рекурсивное сканирование директории */
void scan_directory(const char *path) {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(path);
    if (dir == NULL) {
        perror("Ошибка открытия директории");
        return;
    }

    verbose_log("Сканирование директории: ");
    verbose_log_path(path);

    while ((entry = readdir(dir)) != NULL) {
        // Пропускаем специальные записи "." и ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char full_path[PATH_MAX];
        const char *format = (path[strlen(path) - 1] == '/') ? "%s%s" : "%s/%s";
        snprintf(full_path, sizeof(full_path), format, path, entry->d_name);

        struct stat statbuf;
        if (stat(full_path, &statbuf) < 0) {
            perror("Ошибка получения информации");
            printf("Для: %s\n", full_path);
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            // Если рекурсия разрешена, заходим в поддиректорию
            if (recursive_flag) {
                verbose_log("Найден каталог: ");
                verbose_log_path(full_path);
                scan_directory(full_path);
            }
        } else {
            add_file_entry(full_path, statbuf.st_size);
        }
    }

    closedir(dir);
}
