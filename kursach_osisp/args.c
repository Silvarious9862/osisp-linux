#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "args.h"

Options parse_arguments(int argc, char *argv[]) {
    Options opts = {0, 0, 0, 0, 0, NULL};
    int index = 1;

    // Обрабатываем все аргументы, начинающиеся с '-' (но не считаем одиночный "-" за опцию)
    while (index < argc && argv[index][0] == '-' && argv[index][1] != '\0') {
        // Если аргумент равен "--", прекращаем разбор опций
        if (strcmp(argv[index], "--") == 0) {
            index++;
            break;
        }

        // Перебираем каждый символ после дефиса
        for (int i = 1; argv[index][i] != '\0'; i++) {
            switch (argv[index][i]) {
                case 'r': opts.recursive = 1;   break;
                case 't': opts.show_time = 1;   break;
                case 'm': opts.summary = 1;     break;
                case 'S': opts.show_size = 1;   break;
                case 'v': opts.verbose = 1;     break;
                default:
                    fprintf(stderr, "Неизвестный флаг: -%c\n", argv[index][i]);
                    exit(EXIT_FAILURE);
            }
        }
        index++;
    }

    if (index >= argc) {
        fprintf(stderr, "Ошибка: не указан начальный путь\n");
        exit(EXIT_FAILURE);
    }
    opts.start_path = argv[index];

    // Проверяем, что указанный путь существует и является директорией
    struct stat statbuf;
    if (stat(opts.start_path, &statbuf) != 0 || !S_ISDIR(statbuf.st_mode)) {
        fprintf(stderr, "Указанный путь некорректен или не является директорией: %s\n", opts.start_path);
        exit(EXIT_FAILURE);
    }

    return opts;
}
