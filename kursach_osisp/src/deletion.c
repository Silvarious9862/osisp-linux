/* deletion.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <limits.h>


/* Определение структуры для хранения информации о файле */
typedef struct {
    char full_path[PATH_MAX];
    off_t file_size;
} file_entry;

/* Глобальные переменные, определённые в selection.c */
extern file_entry *file_list;
extern size_t file_count;
extern size_t file_list_capacity;
extern int time_flag;

/*
 * Функция interactive_delete_duplicates реализует интерактивный режим удаления дубликатов
 * в plain-стиле (аналог fdupes с опциями -dP).
 *
 * Для каждой группы дубликатов (файлы с одинаковым file_size) выводится нумерованный список.
 * Если флаг time_flag установлен, для каждого файла дополнительно выводится время модификации в формате
 * [YYYY-MM-DD HH:MM]. Затем пользователь получает приглашение:
 *
 *   "Set X of Y, preserve files [1 - N, all, quit]: "
 *
 * Если введено неверное значение (например, номер вне диапазона или нечисловой ввод),
 * запрос повторяется до получения корректного ответа.
 * При выборе "all" группа сохраняется, а при "quit" — выход из режима.
 * Иначе, сохраняется выбранный файл, а остальные удаляются с помощью remove().
 */
void interactive_delete_duplicates(void) {
    if (file_count == 0) {
        printf("Нет дубликатов для удаления.\n");
        return;
    }
    
    // Подсчитываем общее число наборов (групп) дубликатов (где файлов >= 2)
    size_t total_sets = 0;
    size_t idx = 0;
    while (idx < file_count) {
        size_t groupCount = 0;
        off_t current_size = file_list[idx].file_size;
        while (idx < file_count && file_list[idx].file_size == current_size) {
            groupCount++;
            idx++;
        }
        if (groupCount >= 2)
            total_sets++;
    }
    
    // Сброс индекса для повторного прохода по списку
    idx = 0;
    size_t set_number = 0;
    
    while (idx < file_count) {
        size_t group_start = idx;
        off_t current_size = file_list[idx].file_size;
        size_t groupCount = 0;
        while (idx < file_count && file_list[idx].file_size == current_size) {
            groupCount++;
            idx++;
        }
        if (groupCount < 2)
            continue;
        
        set_number++;  // Нумеруем группу
        
        // Выводим список файлов в группе
        for (size_t j = 0; j < groupCount; j++) {
            if (time_flag) {
                struct stat sb;
                if (stat(file_list[group_start + j].full_path, &sb) == 0) {
                    char timebuf[32];
                    struct tm *tm_info = localtime(&sb.st_mtime);
                    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M", tm_info);
                    printf("[%zu] [%s] %s\n", j + 1, timebuf, file_list[group_start + j].full_path);
                } else {
                    printf("[%zu] [unknown time] %s\n", j + 1, file_list[group_start + j].full_path);
                }
            } else {
                printf("[%zu] %s\n", j + 1, file_list[group_start + j].full_path);
            }
        }
        
        // Приглашение с повторным вводом до корректного значения
        char input[32];
        int keep_index = -1;
        while (1) {
            printf("\nНабор %zu из %zu, сохранить файл [1 - %zu, all, quit]: ", 
                   set_number, total_sets, groupCount);
            if (!fgets(input, sizeof(input), stdin))
                continue;
            input[strcspn(input, "\n")] = '\0';
            if (strcmp(input, "quit") == 0) {
                printf("Выход из режима удаления.\n");
                return;
            }
            if (strcmp(input, "all") == 0) {
                printf("Группа сохранена (удаление пропущено).\n\n");
                break;
            }
            keep_index = atoi(input) - 1;
            if (keep_index >= 0 && (size_t)keep_index < groupCount)
                break;
            printf("Неверный выбор, попробуйте снова.\n");
        }
        
        // Если пользователь ввёл "all", удаление в группе не производится
        if (strcmp(input, "all") == 0)
            continue;
        
        // Удаляем все файлы группы, кроме выбранного
        for (size_t j = 0; j < groupCount; j++) {
            if (j == (size_t)keep_index)
                continue;
            if (remove(file_list[group_start + j].full_path) == 0)
                printf("Удален: %s\n", file_list[group_start + j].full_path);
            else
                perror("Ошибка удаления");
        }
        printf("\n");
    }
}


void non_interactive_delete_duplicates(void) {
    if (file_count == 0) {
        printf("Нет дубликатов для удаления.\n");
        return;
    }
    
    size_t idx = 0;
    while (idx < file_count) {
        size_t group_start = idx;
        off_t current_size = file_list[idx].file_size;
        size_t groupCount = 0;
        
        // Группируем файлы с одинаковым file_size
        while (idx < file_count && file_list[idx].file_size == current_size) {
            groupCount++;
            idx++;
        }
        
        // Если в группе менее двух файлов, дубликатов нет – пропускаем
        if (groupCount < 2)
            continue;
        
        // Сохраняем первый файл группы
        printf("   [+] %s\n", file_list[group_start].full_path);
        
        // Удаляем все остальные файлы группы
        for (size_t j = group_start + 1; j < group_start + groupCount; j++) {
            if (remove(file_list[j].full_path) == 0)
                printf("   [-] %s\n", file_list[j].full_path);
            else
                perror("Ошибка удаления");
        }
        printf("\n");
    }
}
