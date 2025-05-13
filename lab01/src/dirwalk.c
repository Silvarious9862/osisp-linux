#include "dirwalk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <locale.h>

#define INITIAL_CAPACITY 128

/* Структура для хранения списка найденных путей */
typedef struct {
    char **items;
    size_t size;
    size_t capacity;
} FileList;

static FileList* create_file_list(void) {
    FileList *list = malloc(sizeof(FileList));
    if (!list) {
         perror("malloc");
         exit(EXIT_FAILURE);
    }
    list->capacity = INITIAL_CAPACITY;
    list->size = 0;
    list->items = malloc(list->capacity * sizeof(char *));
    if (!list->items) {
         perror("malloc");
         exit(EXIT_FAILURE);
    }
    return list;
}

static void add_to_list(FileList *list, const char *path) {
    if (list->size >= list->capacity) {
         list->capacity *= 2;
         list->items = realloc(list->items, list->capacity * sizeof(*list->items));
         if (!list->items) {
             perror("realloc");
             exit(EXIT_FAILURE);
         }
    }
    list->items[list->size] = strdup(path);
    if (!list->items[list->size]) {
         perror("strdup");
         exit(EXIT_FAILURE);
    }
    list->size++;
}

static int cmp_str(const void *a, const void *b) {
    const char *str_a = *(const char **)a;
    const char *str_b = *(const char **)b;
    return strcoll(str_a, str_b);
}

static void free_file_list(FileList *list) {
    for (size_t i = 0; i < list->size; i++) {
         free(list->items[i]);
    }
    free(list->items);
    free(list);
}

static void print_file_list(const FileList *list) {
    for (size_t i = 0; i < list->size; i++) {
         printf("%s\n", list->items[i]);
    }
}

/* Функция фильтрации – проверяет, соответствует ли файл заданным опциям */
static int match_type(const char *path, const Options *opts) {
    struct stat sb;
    if (lstat(path, &sb) == -1) {
        perror(path);
        return 0;
    }
    /* Если ни одна из опций не установлена, выводим все типы объектов */
    if (!opts->type_files && !opts->type_dirs && !opts->type_links)
         return 1;

    if (opts->type_files && S_ISREG(sb.st_mode))
         return 1;
    if (opts->type_dirs && S_ISDIR(sb.st_mode))
         return 1;
    if (opts->type_links && S_ISLNK(sb.st_mode))
         return 1;
    
    return 0;
}

/* Внутренняя функция обхода каталога */
static void dirwalk_internal(const char *dir, const Options *opts, FileList *list) {
    DIR *dp = opendir(dir);
    if (dp == NULL) {
         perror(dir);
         return;
    }
    struct dirent *entry;
    while ((entry = readdir(dp)) != NULL) {
         if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
               continue;

         /* Вычисляем длину строки для выделения памяти с учетом необходимости слэша */
         size_t dir_len = strlen(dir);
         int need_slash = (dir[dir_len - 1] == '/') ? 0 : 1;
         size_t path_len = dir_len + need_slash + strlen(entry->d_name) + 1; // +1 для нулевого символа

         char *path = malloc(path_len);
         if (path == NULL) {
               perror("malloc");
               closedir(dp);
               exit(EXIT_FAILURE);
         }

         /* Формируем строку пути с условием добавления слэша */
         if (need_slash)
             snprintf(path, path_len, "%s/%s", dir, entry->d_name);
         else
             snprintf(path, path_len, "%s%s", dir, entry->d_name);

         if (match_type(path, opts)) {
               add_to_list(list, path);
         }

         /* Если объект – каталог, заходим в него рекурсивно */
         struct stat sb;
         if (lstat(path, &sb) == 0 && S_ISDIR(sb.st_mode)) {
              dirwalk_internal(path, opts, list);
         }
         free(path);
    }
    closedir(dp);
}

void dirwalk(const char *start_dir, const Options *opts) {
    /* Устанавливаем локаль для корректной сортировки, если задана опция -s */
    if (opts->sort) {
         setlocale(LC_COLLATE, "");
    }
    
    FileList *list = create_file_list();
    dirwalk_internal(start_dir, opts, list);
    
    if (opts->sort) {
         qsort(list->items, list->size, sizeof(char *), cmp_str);
    }
    
    print_file_list(list);
    free_file_list(list);
}
