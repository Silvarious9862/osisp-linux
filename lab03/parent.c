#define _POSIX_C_SOURCE 200809L // Активация POSIX API для C11
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_CHILDREN 100

pid_t children[MAX_CHILDREN]; // Массив для хранения PID дочерних процессов
int child_count = 0;          // Текущее количество дочерних процессов

void add_child() {
    if (child_count >= MAX_CHILDREN) {
        printf("Достигнуто максимальное количество дочерних процессов.\n");
        return;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("Ошибка при fork");
        exit(1);
    } else if (pid == 0) {
        // Дочерний процесс
        execl("./child", "./child", NULL); // Запускаем программу "child"
        perror("Ошибка при запуске дочернего процесса");
        exit(1);
    } else {
        // Родительский процесс
        children[child_count++] = pid;
        printf("Создан дочерний процесс с PID %d.\n", pid);
    }
}

void remove_last_child() {
    if (child_count == 0) {
        printf("Нет активных дочерних процессов для завершения.\n");
        return;
    }

    pid_t pid = children[--child_count];
    kill(pid, SIGTERM); // Отправляем сигнал завершения
    waitpid(pid, NULL, 0); // Ждем завершения
    printf("Завершен дочерний процесс с PID %d. Осталось процессов: %d.\n", pid, child_count);
}

void list_processes() {
    printf("Родительский процесс PID %d.\n", getpid());
    for (int i = 0; i < child_count; i++) {
        printf("Дочерний процесс PID %d.\n", children[i]);
    }
}

void kill_all_children() {
    for (int i = 0; i < child_count; i++) {
        kill(children[i], SIGTERM);
        waitpid(children[i], NULL, 0);
        printf("Завершен дочерний процесс с PID %d.\n", children[i]);
    }
    child_count = 0;
}

void handle_input() {
    char command;
    while (1) {
        command = getchar();
        switch (command) {
            case '+':
                add_child();
                break;
            case '-':
                remove_last_child();
                break;
            case 'l':
                list_processes();
                break;
            case 'k':
                kill_all_children();
                break;
            case 'q':
                kill_all_children();
                printf("Родительский процесс завершен.\n");
                exit(0);
            default:
                break;
        }
    }
}

int main() {
    printf("Введите команду: '+' для создания, '-' для удаления,"
        "'l' для списка, 'k' для завершения всех, 'q' для выхода.\n");
    handle_input();
    return 0;
}
