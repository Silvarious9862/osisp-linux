#define _POSIX_C_SOURCE 200809L // Активация POSIX API для C11
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    int x;
    int y;
} Data;

volatile sig_atomic_t sigusr1_triggered = 0; // Флаг для обработки сигнала
int combinations[4] = {0, 0, 0, 0};          // Счётчики для статистики

// Обработчик сигнала SIGUSR1
void sigusr1_handler(int sig) {
    if (sig == SIGUSR1) {
        //printf("Обработан сигнал SIGUSR1\n");
        sigusr1_triggered = 1;
    }
}

// Функция для обработки данных
void process_data(Data *data) {
    int index = (data->x << 1) | data->y; // Вычисление индекса комбинации
    if (index >= 0 && index < 4) {
        combinations[index]++;
    }
}

// Функция для вывода статистики
void print_statistics(pid_t ppid, pid_t pid) {
    printf("PPID: %d, PID: %d, {0,0}: %d, {1,1}: %d, {0,1}: %d, {1,0}: %d\n",
           ppid, pid, combinations[0], combinations[1], combinations[2], combinations[3]);
}

int main() {
    // Установка обработчика для SIGUSR1 с использованием sigaction
    struct sigaction sa;
    sa.sa_handler = sigusr1_handler;  // Указываем функцию-обработчик
    sa.sa_flags = 0;                 // Без дополнительных флагов
    sigemptyset(&sa.sa_mask);        // Очищаем маску сигналов

    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("Ошибка установки обработчика SIGUSR1");
        exit(EXIT_FAILURE);
    }

    // Инициализация генератора случайных чисел
    srand(time(NULL));

    Data data = {0, 0}; // Структура для хранения текущих данных
    int iterations = 0; // Счётчик итераций

    while (1) {
        // Устанавливаем интервал ожидания (2 секунды)
        struct timespec ts = {0, 100000000};
        nanosleep(&ts, NULL);

        data.x = rand() % 2;
        data.y = rand() % 2;

        // Генерируем сигнал SIGUSR1 самому себе
        raise(SIGUSR1);

        // Обрабатываем данные, если сигнал активирован
        if (sigusr1_triggered) {
            sigusr1_triggered = 0;

            process_data(&data); // Собираем статистику
            iterations++;

            // Печатаем статистику каждые 101 итерацию
            if (iterations % 101 == 0) {
                print_statistics(getppid(), getpid());
            }
        }
    }

    return 0;
}
