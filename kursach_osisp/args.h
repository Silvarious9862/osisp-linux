#ifndef ARGS_H
#define ARGS_H

typedef struct {
    int recursive;   
    int show_time;   
    int summary;
    int show_size;
    int verbose;
    int deletion;
    int no_interact;
    unsigned long long max_size;
    unsigned long long min_size;
    const char *start_path; // Начальный путь для сканирования
} Options;

/**
 * Функция разбора аргументов командной строки.
 * Принимает argc и argv, возвращает структуру Options с установленными значениями.
 * Завершает программу, если ошибки параметров обнаружены.
 */
Options parse_arguments(int argc, char *argv[]);

#endif  // ARGS_H
