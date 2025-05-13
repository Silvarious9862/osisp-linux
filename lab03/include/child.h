#ifndef CHILD_H
#define CHILD_H

#include <sys/types.h>
#include <signal.h>

// Структура для хранения данных
typedef struct {
    int x;
    int y;
} Data;

// Флаги и статистика (объявлены как extern, чтобы использовать в разных файлах при необходимости)
extern volatile sig_atomic_t sigusr1_triggered;
// Флаг, который будет выставлен при получении сигнала SIGUSR2 («разрешение» на вывод статистики)
extern volatile sig_atomic_t print_permission;
extern int combinations[4];

// Прототипы функций для обработки сигналов и данных
void sigusr1_handler(int sig);
void sigusr2_handler(int sig);
void process_data(Data *data);
void print_statistics(pid_t ppid, pid_t pid);

#endif /* CHILD_H */
