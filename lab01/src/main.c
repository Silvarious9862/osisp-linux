#include <stdio.h>
#include <stdlib.h>
#include "options.h"
#include "dirwalk.h"

int main(int argc, char *argv[]) {
    char *start_dir;
    Options opts;

    /* Разбор опций командной строки */
    parse_options(argc, argv, &start_dir, &opts);

    /* Запуск рекурсивного обхода */
    dirwalk(start_dir, &opts);

    return 0;
}
