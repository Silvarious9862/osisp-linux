#include "spawn_children.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

// Внешнее объявление глобальной переменной окружения
extern char **environ;

void spawn_child_plus(int child_number, const char *env_file) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("Ошибка fork");
        exit(1);
    } else if (pid == 0) {
        // Дочерний процесс для режима '+'
        char child_name[20];
        snprintf(child_name, sizeof(child_name), "child_%02d", child_number);

        char *child_path = getenv("CHILD_PATH");
        if (!child_path) {
            fprintf(stderr, "Ошибка: переменная окружения CHILD_PATH не задана.\n");
            exit(1);
        }

        char child_exec[512];
        snprintf(child_exec, sizeof(child_exec), "%s/child", child_path);

        // Передаем путь к файлу env как второй аргумент
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
        // Дочерний процесс для режима '*'
        char child_name[20];
        snprintf(child_name, sizeof(child_name), "child_%02d", child_number);

        char *child_path = getenv("CHILD_PATH");
        if (!child_path) {
            fprintf(stderr, "Ошибка: переменная окружения CHILD_PATH не задана.\n");
            exit(1);
        }

        char child_exec[512];
        snprintf(child_exec, sizeof(child_exec), "%s/child", child_path);

        // Формируем сокращенное окружение и добавляем переменную CHILD_MODE для идентификации режима '*'
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
            "CHILD_MODE=star",
            NULL
        };

        // Передаем NULL как второй аргумент — файл env не нужен в режиме '*'
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
        // Дочерний процесс для режима '&'
        char child_name[20];
        snprintf(child_name, sizeof(child_name), "child_%02d", child_number);

        char *child_path = getenv("CHILD_PATH");
        if (!child_path) {
            fprintf(stderr, "Ошибка: переменная окружения CHILD_PATH не задана.\n");
            exit(1);
        }

        char child_exec[512];
        snprintf(child_exec, sizeof(child_exec), "%s/child", child_path);

        // Передаём стандартное окружение без переменной CHILD_MODE
        char *args[] = {child_name, NULL};

        execve(child_exec, args, environ);
        perror("Ошибка execve");
        exit(1);
    } else {
        printf("Запущен дочерний процесс %d с PID %d (режим '&')\n", child_number, pid);
    }
}
