#ifndef VERBOSE_H
#define VERBOSE_H

#include <stdio.h>

// ANSI escape-коды для цветов
#define BLUE   "\033[1;34m"  // Синий цвет
#define GREEN  "\033[1;32m"  // Зеленый цвет
#define RESET  "\033[0m"     // Сброс цвета

extern int verbose_flag;

static void verbose_log(const char *message) {
    if (verbose_flag) {
        printf(BLUE "[verbose] %s\n" RESET, message);
    }
}

#define MAX_VERBOSE_PATH_LENGTH 50  // максимальная длина для вывода

// Функция печати verbose-сообщения с обрезкой начала пути, если он слишком длинный.
static void verbose_log_path(const char *full_path) {
    if (verbose_flag) {
        char truncated_path[MAX_VERBOSE_PATH_LENGTH + 1];
        size_t len = strlen(full_path);
        if (len > MAX_VERBOSE_PATH_LENGTH) {
            // Оставляем место для троеточия ("...") – 3 символа.
            size_t tail_len = MAX_VERBOSE_PATH_LENGTH - 3;
            // Копируем конец full_path, начиная с позиции, чтобы итоговая длина была MAX_VERBOSE_PATH_LENGTH.
            snprintf(truncated_path, sizeof(truncated_path), "...%s", full_path + len - tail_len);
        } else {
            // Если путь укладывается в лимит, просто копируем его
            snprintf(truncated_path, sizeof(truncated_path), "%s", full_path);
        }
        // Выводим сообщение с выбранным цветом (синий) через ANSI-коды
        printf(BLUE "[verbose] " GREEN "%s\n" RESET, truncated_path);
    }
}

#endif // VERBOSE_H