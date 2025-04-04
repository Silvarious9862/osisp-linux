#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

// Внешнее объявление переменных окружения
extern char **environ;

void spawn_child_plus(int child_number, const char *env_file) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("Ошибка fork");
        exit(1);
    } else if (pid == 0) {
        // Дочерний процесс для '+'
        char child_name[20];
        snprintf(child_name, sizeof(child_name), "child_%02d", child_number);

        char *child_path = getenv("CHILD_PATH");
        if (!child_path) {
            fprintf(stderr, "Ошибка: переменная окружения CHILD_PATH не задана.\n");
            exit(1);
        }

        char child_exec[512];
        snprintf(child_exec, sizeof(child_exec), "%s/child", child_path);

        // Передаем путь к env как второй аргумент
        char *args[] = {child_name, (char *)env_file, NULL};
        execve(child_exec, args, environ);

        perror("Ошибка execve");
        exit(1);
    } else {
        printf("Запущен дочерний процесс %d с PID %d (режим '+')\n", child_number, pid);
    }
}

void spawn_child_star(int child_number) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("Ошибка fork");
        exit(1);
    } else if (pid == 0) {
        // Дочерний процесс для '*'
        char child_name[20];
        snprintf(child_name, sizeof(child_name), "child_%02d", child_number);

        char *child_path = getenv("CHILD_PATH");
        if (!child_path) {
            fprintf(stderr, "Ошибка: переменная окружения CHILD_PATH не задана.\n");
            exit(1);
        }

        char child_exec[512];
        snprintf(child_exec, sizeof(child_exec), "%s/child", child_path);

        // Формируем сокращенное окружение
        char *child_env[] = {
            "SHELL=/bin/bash",
            "HOME=/home/silvarious",
            "HOSTNAME=fedora",
            "LOGNAME=silvarious",
            "LANG=en_US.UTF-8",
            "TERM=xterm-256color",
            "USER=silvarious",
            "LC_COLLATE=C",
            "PATH=/usr/bin:/bin:/usr/sbin:/sbin",
            NULL
        };

        // Передаем массив `envp` с сокращенным окружением
        char *args[] = {child_name, NULL};
        execve(child_exec, args, child_env);

        perror("Ошибка execve");
        exit(1);
    } else {
        printf("Запущен дочерний процесс %d с PID %d (режим '*')\n", child_number, pid);
    }
}

void spawn_child_amp(int child_number) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("Ошибка fork");
        exit(1);
    } else if (pid == 0) {
        // Дочерний процесс для '&'
        char child_name[20];
        snprintf(child_name, sizeof(child_name), "child_%02d", child_number);

        char *child_path = getenv("CHILD_PATH");
        if (!child_path) {
            fprintf(stderr, "Ошибка: переменная окружения CHILD_PATH не задана.\n");
            exit(1);
        }

        char child_exec[512];
        snprintf(child_exec, sizeof(child_exec), "%s/child", child_path);

        // Передаем стандартное окружение
        char *args[] = {child_name, NULL};
        execve(child_exec, args, environ);

        perror("Ошибка execve");
        exit(1);
    } else {
        printf("Запущен дочерний процесс %d с PID %d (режим '&')\n", child_number, pid);
    }
}

int main() {
    int child_count = 0; // Счетчик дочерних процессов
    printf("PID: %d\n", getpid());
    printf("Введите '+', '*', '&' для порождения процесса, 'q' для выхода:\n");

    char input;
    while ((input = getchar()) != EOF) {
        if (input == '+') {
            spawn_child_plus(child_count++, "env");
        } else if (input == '*') {
            spawn_child_star(child_count++);
        } else if (input == '&') {
            spawn_child_amp(child_count++);
        } else if (input == 'q') {
            printf("Родительский процесс завершает работу.\n");
            break;
        }
        // Пропускаем символ новой строки
        if (input != '\n') {
            while (getchar() != '\n');
        }
    }

    return 0;
}
