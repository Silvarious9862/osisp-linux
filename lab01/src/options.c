#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "options.h"

static void usage(const char *progname) {
    fprintf(stderr, "Usage: %s [dir] [options]\n", progname);
    exit(EXIT_FAILURE);
}

void parse_options(int argc, char *argv[], char **start_dir, Options *opts) {
    // Инициализируем все флаги нулями
    opts->type_links = 0;
    opts->type_dirs  = 0;
    opts->type_files = 0;
    opts->sort       = 0;

    int opt;
    while ((opt = getopt(argc, argv, "ldfs")) != -1) {
        switch (opt) {
            case 'l':
                opts->type_links = 1;
                break;
            case 'd':
                opts->type_dirs = 1;
                break;
            case 'f':
                opts->type_files = 1;
                break;
            case 's':
                opts->sort = 1;
                break;
            default:
                usage(argv[0]);
        }
    }
    // Если остался аргумент, интерпретируем его как начальный каталог
    if (optind < argc) {
        *start_dir = argv[optind];
    } else {
        *start_dir = "./";
    }
}
