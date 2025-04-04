#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

void dirwalk(const char *base_path, const char *current_path, int show_files, int show_dirs, int show_links) {
    struct dirent *entry;
    struct stat statbuf;
    DIR *dp = opendir(current_path);

    if (dp == NULL) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dp)) != NULL) {
        char fullpath[PATH_MAX];
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue; // Пропускаем "." и ".."
        }

        snprintf(fullpath, sizeof(fullpath), "%s/%s", current_path, entry->d_name);

        if (lstat(fullpath, &statbuf) == -1) {
            perror("lstat");
            continue;
        }

        // Проверяем тип элемента и фильтруем в зависимости от флагов
        if (S_ISREG(statbuf.st_mode) && show_files) {
            // Если это файл и выбран флаг -f, выводим файл
            printf("%s\n", fullpath + strlen(base_path) + 1);
        }

        if (S_ISLNK(statbuf.st_mode) && show_links) {
            // Если это символическая ссылка и выбран флаг -l, выводим ссылку
            printf("%s [symlink]\n", fullpath + strlen(base_path) + 1);
        }

        // В любом случае (независимо от флага -d) продолжаем рекурсию в каталоги
        if (S_ISDIR(statbuf.st_mode)) {
            if (show_dirs) {
                // Если выбран флаг -d, выводим каталоги
                printf("%s/\n", fullpath + strlen(base_path) + 1);
            }
            dirwalk(base_path, fullpath, show_files, show_dirs, show_links); // Рекурсивно обходим подкаталог
        }
    }

    closedir(dp);
}

int main(int argc, char *argv[]) {
    const char *start_path = "./"; // Стартовый путь по умолчанию
    int show_files = 0, show_dirs = 0, show_links = 0; // Флаги фильтров
    int option;

    // Используем getopt для обработки флагов
    while ((option = getopt(argc, argv, "fdl")) != -1) {
        switch (option) {
            case 'f':
                show_files = 1;
                break;
            case 'd':
                show_dirs = 1;
                break;
            case 'l':
                show_links = 1;
                break;
            default:
                fprintf(stderr, "Неизвестный флаг: -%c\n", optopt);
                return 1;
        }
    }

    // Определяем путь, если он указан
    if (optind < argc) {
        start_path = argv[optind];
    }

    // Если флаги фильтров не указаны, выводить все типы
    if (!show_files && !show_dirs && !show_links) {
        show_files = show_dirs = show_links = 1;
    }

    // Выводим результаты (пример демонстрации логики)
    /*printf("Путь: %s\n", start_path ? start_path : "не указан");
    printf("Показывать файлы: %d\n", show_files);
    printf("Показывать директории: %d\n", show_dirs);
    printf("Показывать ссылки: %d\n", show_links);*/

    char real_base[PATH_MAX];
    if (realpath(start_path, real_base) == NULL) {
        perror("realpath");
        return 1;
    }

    dirwalk(real_base, real_base, show_files, show_dirs, show_links); // Запуск обхода
    return 0;
}
