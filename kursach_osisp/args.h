#ifndef ARGS_H
#define ARGS_H

typedef struct {
    int recursive;   // Флаг для рекурсивного обхода (-r)
    int show_time;   // Флаг для вывода времени последней модификации (-t)
    int summary;
    int show_size;
    int verbose;
    const char *start_path; // Начальный путь для сканирования
} Options;

/**
 * Функция разбора аргументов командной строки.
 * Принимает argc и argv, возвращает структуру Options с установленными значениями.
 * Завершает программу, если ошибки параметров обнаружены.
 */
Options parse_arguments(int argc, char *argv[]);

#endif  // ARGS_H
