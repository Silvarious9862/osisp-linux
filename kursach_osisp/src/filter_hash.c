#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <openssl/md5.h>
#include "verbose.h"

/* Определение структуры file_entry */
typedef struct {
    char full_path[PATH_MAX];
    off_t file_size;
} file_entry;

/* Объявление глобальных переменных */
extern file_entry *file_list;
extern size_t file_count;
extern size_t file_list_capacity;

/* Вспомогательная структура для хранения записи и её MD5-хеша */
typedef struct {
    file_entry entry;
    char *hash;
} file_hash;

/* Функция для вычисления MD5-хеша файла.
   Возвращает выделенную строку с 32-символьным хешем (в шестнадцатеричном представлении);
   в случае ошибки возвращается NULL. */
static char *compute_md5(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("не удалось открыть файл для чтения");
        return NULL;
    }

    MD5_CTX ctx;
    if (MD5_Init(&ctx) != 1) {
        fclose(fp);
        fprintf(stderr, "ошибка инициализации MD5\n");
        return NULL;
    }

    unsigned char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        if (MD5_Update(&ctx, buffer, bytes) != 1) {
            fclose(fp);
            fprintf(stderr, "ошибка вычисления MD5\n");
            return NULL;
        }
    }
    fclose(fp);

    unsigned char digest[MD5_DIGEST_LENGTH];
    if (MD5_Final(digest, &ctx) != 1) {
        fprintf(stderr, "ошибка завершения вычисления MD5\n");
        return NULL;
    }

    char *md5_str = malloc(MD5_DIGEST_LENGTH * 2 + 1);
    if (!md5_str) {
        perror("ошибка выделения памяти для MD5 строки");
        return NULL;
    }
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        sprintf(&md5_str[i * 2], "%02x", digest[i]);
    }
    md5_str[MD5_DIGEST_LENGTH * 2] = '\0';
    return md5_str;
}

/* Функция сравнения для qsort, сравнивает MD5-хеши двух файлов */
static int compare_file_hash(const void *a, const void *b) {
    const file_hash *fa = (const file_hash *)a;
    const file_hash *fb = (const file_hash *)b;
    return strcmp(fa->hash, fb->hash);
}

/* Функция filter_hash_list:
   На основе глобального массива file_list вычисляет MD5 для каждого файла,
   сортирует записи по хешу и группирует файлы с одинаковым MD5.
   Если для определённого хеша найден только один файл, он не включается в итоговый список.
   После фильтрации старый список освобождается, а file_list обновляется новым списком. */
void filter_hash_list(void) {
    if (file_count == 0)
        return;

    file_hash *hash_array = malloc(file_count * sizeof(file_hash));
    if (!hash_array) {
        perror("ошибка выделения памяти для hash_array");
        exit(EXIT_FAILURE);
    }

    /* Вычисляем MD5-хеш для каждого файла */
    for (size_t i = 0; i < file_count; i++) {
        hash_array[i].entry = file_list[i];
        hash_array[i].hash = compute_md5(file_list[i].full_path);
        if (!hash_array[i].hash) {
            /* Если не удалось вычислить, устанавливаем пустую строку (такой файл, скорее всего, исключится) */
            hash_array[i].hash = strdup("");
        }
    }

    /* Сортируем массив по MD5-хешу */
    qsort(hash_array, file_count, sizeof(file_hash), compare_file_hash);

    file_entry *filtered_list = NULL;
    size_t filtered_count = 0;
    size_t filtered_capacity = 0;
    int unique_flag = 0;
    verbose_log("Уникальные файлы:");

    size_t i = 0;
    while (i < file_count) {
        size_t j = i + 1;
        while (j < file_count && strcmp(hash_array[j].hash, hash_array[i].hash) == 0)
            j++;

        /* Если группа с одинаковым MD5 больше одного файла, добавляем их в итоговый список */
        if ((j - i) > 1) {
            for (size_t k = i; k < j; k++) {
                if (filtered_count >= filtered_capacity) {
                    filtered_capacity = filtered_capacity ? filtered_capacity * 2 : 10;
                    filtered_list = realloc(filtered_list, filtered_capacity * sizeof(file_entry));
                    if (!filtered_list) {
                        perror("ошибка выделения памяти для filtered_list");
                        exit(EXIT_FAILURE);
                    }
                }
                filtered_list[filtered_count++] = hash_array[k].entry;
            }
        } else {
            verbose_log_path(file_list[i].full_path);
            unique_flag = 1;
        }
        i = j;
    }
    if (!unique_flag) verbose_log("не найдены");

    /* Освобождаем все вычисленные MD5-хеш строки и временный массив */
    for (size_t i = 0; i < file_count; i++) {
        free(hash_array[i].hash);
    }
    free(hash_array);

    free(file_list);
    file_list = filtered_list;
    file_count = filtered_count;
    file_list_capacity = filtered_capacity;
}