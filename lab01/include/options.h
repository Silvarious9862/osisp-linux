#ifndef OPTIONS_H
#define OPTIONS_H

typedef struct {
    int type_links; // Флаг для опции -l (символические ссылки)
    int type_dirs;  // Флаг для опции -d (только каталоги)
    int type_files; // Флаг для опции -f (только обычные файлы)
    int sort;       // Флаг для опции -s (сортировка LC_COLLATE)
} Options;

void parse_options(int argc, char *argv[], char **start_dir, Options *opts);

#endif // OPTIONS_H
