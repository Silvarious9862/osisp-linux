#include "parent.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

// Инициализация массива для дочерних процессов и счётчика
pid_t children[MAX_CHILDREN];
int child_count = 0;

void add_child(void) {
    if (child_count >= MAX_CHILDREN) {
        printf("Достигнуто максимальное количество дочерних процессов.\n");
        return;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("Ошибка при fork");
        exit(1);
    } else if (pid == 0) {
        // В дочернем процессе запускаем программу child
        execl("./child", "./child", NULL);
        perror("Ошибка при запуске дочернего процесса");
        exit(1);
    } else {
        children[child_count++] = pid;
        printf("Создан дочерний процесс с PID %d.\n", pid);
    }
}

void remove_last_child(void) {
    if (child_count == 0) {
        printf("Нет активных дочерних процессов для завершения.\n");
        return;
    }

    pid_t pid = children[--child_count];
    kill(pid, SIGTERM);
    waitpid(pid, NULL, 0);
    printf("Завершен дочерний процесс с PID %d. Осталось процессов: %d.\n", pid, child_count);
}

void list_processes(void) {
    printf("Родительский процесс PID %d.\n", getpid());
    for (int i = 0; i < child_count; i++) {
        printf("Дочерний процесс PID %d.\n", children[i]);
    }
}

void kill_all_children(void) {
    for (int i = 0; i < child_count; i++) {
        kill(children[i], SIGTERM);
        waitpid(children[i], NULL, 0);
        printf("Завершен дочерний процесс с PID %d.\n", children[i]);
    }
    child_count = 0;
}

/*  
  Для упорядочения вывода можно добавить дополнительную команду. Например,
  при вводе символа 'o' родительский процесс по очереди отправляет сигнал SIGUSR2
  всем дочерним процессам, позволяя им разблокироваться и напечатать статистику.
  Если нужное упорядочение выводов не достигнуто автоматически, эта команда даст
  возможность «разрешить» вывод ожидающих детей.
*/
void handle_input(void) {
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
            case 'o': {
                // Отправляем SIGUSR2 дочерним процессам по порядку
                for (int i = 0; i < child_count; i++) {
                    kill(children[i], SIGUSR2);
                    // Небольшая задержка для упорядочивания вывода (50 мс)
                    usleep(50000);
                }
                break;
            }
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
    printf("Введите команду:\n"
           "  '+' для создания дочернего процесса,\n"
           "  '-' для удаления последнего дочернего процесса,\n"
           "  'l' для вывода списка процессов,\n"
           "  'o' для упорядоченного вывода статистики дочерних процессов,\n"
           "  'k' для завершения всех дочерних процессов,\n"
           "  'q' для выхода.\n");
    handle_input();
    return 0;
}
