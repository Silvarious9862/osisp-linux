#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include "common.h"

#define MAX_PRODUCERS 100
#define MAX_CONSUMERS 100

volatile sig_atomic_t terminate_flag = 0; // Флаг завершения

pid_t producer_pids[MAX_PRODUCERS];
pid_t consumer_pids[MAX_CONSUMERS];
int producer_count = 0;
int consumer_count = 0;

MessageQueue* queue;
int shm_id;
int sem_id;

// Вспомогательная функция для контроля дедлоков
void check_deadlock() {
    if (producer_count == 0 && queue->free_slots == QUEUE_SIZE) {
        printf("---Нет активных производителей! Нужно добавить производителей.---\n");
    }
    if (consumer_count == 0 && queue->free_slots == 0) {
        printf("---Нет активных потребителей! Нужно добавить потребителей.---\n");
    }
}

// Поток для периодической проверки дедлоков
void* deadlock_monitor(void* arg) {
    while (!terminate_flag) {
        check_deadlock();
        sleep(5);
    }
    return NULL;
}

// Инициализация общей памяти и семафорного набора
void init_resources() {
    shm_id = shmget(SHM_KEY, sizeof(MessageQueue), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("Ошибка при создании общей памяти");
        exit(1);
    }
    queue = (MessageQueue*) shmat(shm_id, NULL, 0);
    if (queue == (void*) -1) {
        perror("Ошибка подключения к общей памяти");
        exit(1);
    }
    memset(queue, 0, sizeof(MessageQueue));
    queue->free_slots = QUEUE_SIZE;

    sem_id = semget(SEM_KEY, 3, IPC_CREAT | 0666);
    if (sem_id == -1) {
        perror("Ошибка при создании семафоров");
        exit(1);
    }
    // Инициализация семафоров:
    // [0] — свободные слоты, [1] — заполненные слоты, [2] — мьютекс
    semctl(sem_id, 0, SETVAL, QUEUE_SIZE);
    semctl(sem_id, 1, SETVAL, 0);
    semctl(sem_id, 2, SETVAL, 1);
}

// Очистка ресурсов и отправка сигнала завершения дочерним процессам
void cleanup_resources() {
    terminate_flag = 1;
    for (int i = 0; i < producer_count; i++) {
        kill(producer_pids[i], SIGTERM);
    }
    for (int i = 0; i < consumer_count; i++) {
        kill(consumer_pids[i], SIGTERM);
    }
    shmdt(queue);
    shmctl(shm_id, IPC_RMID, NULL);
    semctl(sem_id, 0, IPC_RMID);
}

// Вывод текущего состояния очереди
void print_status() {
    printf("Состояние очереди:\n");
    printf("Добавлено сообщений: %d\n", queue->added_count);
    printf("Извлечено сообщений: %d\n", queue->removed_count);
    printf("Свободные места: %d\n", queue->free_slots);
    printf("Занятые места: %d\n", QUEUE_SIZE - queue->free_slots);
    printf("Производителей: %d, Потребителей: %d\n", producer_count, consumer_count);
}

// Обработка команд пользователя
void handle_commands() {
    char command[10];
    printf("Команды: '+' для запуска производителя, '-' для запуска потребителя, 'p' для просмотра состояния, 'q' для выхода.\n");
    while (1) {
        if (fgets(command, sizeof(command), stdin) != NULL) {
            switch (command[0]) {
                case '+': {
                    pid_t pid = fork();
                    if (pid == 0) {
                        execl("./producer", "./producer", NULL);
                        perror("Ошибка при запуске производителя");
                        exit(1);
                    } else {
                        producer_pids[producer_count++] = pid;
                    }
                    break;
                }
                case '-': {
                    pid_t pid = fork();
                    if (pid == 0) {
                        execl("./consumer", "./consumer", NULL);
                        perror("Ошибка при запуске потребителя");
                        exit(1);
                    } else {
                        consumer_pids[consumer_count++] = pid;
                    }
                    break;
                }
                case 'p':
                    print_status();
                    break;
                case 'q':
                    cleanup_resources();
                    printf("Завершение программы.\n");
                    exit(0);
                default:
                    printf("Неизвестная команда: %s", command);
                    break;
            }
        } else {
            printf("Ошибка ввода команды. Попробуйте снова.\n");
        }
    }
}

int main() {
    printf("Запуск основного процесса.\n");
    init_resources();

    pthread_t monitor_thread;
    pthread_create(&monitor_thread, NULL, deadlock_monitor, NULL);

    handle_commands();

    pthread_join(monitor_thread, NULL);
    return 0;
}
