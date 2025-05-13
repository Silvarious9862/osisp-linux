#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "spawn_children.h"

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
        // Пропускаем оставшиеся символы текущей строки
        if (input != '\n') {
            while (getchar() != '\n');
        }
    }
    return 0;
}
