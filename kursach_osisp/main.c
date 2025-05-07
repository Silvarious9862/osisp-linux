#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include "args.h"
#include "result.h"
#include "verbose.h"

// Прототипы функций, определённых в других модулях (selection.c и filter_size.c)
void scan_directory(const char *path);
void filter_file_list(void);
void filter_mime_list(void);
void filter_hash_list(void);
void filter_cmp_list(void);
void print_filtered_file_list(void);

typedef void (*filter_func_t)(void);
typedef struct {
    const char *message;
    filter_func_t func;
} Filter;
Filter filters[] = {
    {"Фильтрация по объему памяти...", filter_file_list},
    {"Фильтрация по типу файла...", filter_mime_list},
    {"Фильтрация по хешу...", filter_hash_list},
    {"Фильтрация побайтовым сравнением...", filter_cmp_list}
};
size_t num_filters = sizeof(filters) / sizeof(filters[0]);

int recursive_flag = 0;
int time_flag = 0;
int summary_flag = 0;
int size_flag = 0;
int verbose_flag = 0;

int main(int argc, char *argv[])
{
    Options opts = parse_arguments(argc, argv);
    recursive_flag = opts.recursive;
    time_flag = opts.show_time;
    summary_flag = opts.summary;
    size_flag = opts.show_size;
    verbose_flag = opts.verbose;

    scan_directory(opts.start_path);

    for (size_t i = 0; i < num_filters; i++) {
        if (file_count == 0) {
            verbose_log("Список файлов пуст, последующие фильтры пропущены.");
            break;
        }
        verbose_log(filters[i].message);
        filters[i].func();
    }

    if (time_flag && size_flag) print_size_time_listing();
    else if (summary_flag) print_summary();
    else if (size_flag) print_size_listing();
    else print_filtered_file_list();

    return EXIT_SUCCESS;
}
