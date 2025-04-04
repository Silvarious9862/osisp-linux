#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern char **environ; // Глобальная переменная окружения

void handle_plus_mode(const char *env_file) {
    printf("\n[Режим '+']: Чтение переменных из файла %s и использование getenv()\n", env_file);

    FILE *file = fopen(env_file, "r");
    if (!file) {
        perror("Ошибка открытия файла");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0'; // Убираем символ новой строки
        char *value = getenv(line);
        if (value) {
            printf("%s=%s\n", line, value);
        } else {
            printf("%s=<не найдено>\n", line);
        }
    }

    fclose(file);
}

void handle_star_mode(char *envp[]) {
    printf("\n[Режим '*']: Использование массива envp\n");

    for (char **env = envp; *env != NULL; env++) {
        printf("%s\n", *env);
    }
}

void handle_amp_mode() {
    printf("\n[Режим '&']: Использование массива environ\n");

    for (char **env = environ; *env != NULL; env++) {
        printf("%s\n", *env);
    }
}

int main(int argc, char *argv[], char *envp[]) {
    // Вывод информации о процессе
    printf("Имя программы: %s\n", argv[0]);
    printf("PID: %d\n", getpid());
    printf("PPID: %d\n", getppid());

    // Автоматическое определение режима работы
    if (argc > 1) {
        // Если передан файл env, работаем в режиме '+'
        handle_plus_mode(argv[1]);
    } else if (envp && envp[0] != NULL) {
        // Если envp передан, работаем в режиме '*'
        handle_star_mode(envp);
    } else {
        // Если ничего не передано, работаем в режиме '&'
        handle_amp_mode();
    }

    printf("\nДочерний процесс завершен.\n");
    return 0;
}
