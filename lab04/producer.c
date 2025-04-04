#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#define SHM_KEY 0x1234
#define SEM_KEY 0x5678
#define QUEUE_SIZE 10

typedef struct {
    char type;
    unsigned short hash;
    unsigned char size;
    char data[256];
} Message;

typedef struct {
    Message buffer[QUEUE_SIZE];
    int head;
    int tail;
    int added_count;
    int removed_count;
    int free_slots;
} MessageQueue;

volatile sig_atomic_t terminate_flag = 0;

void termination_handler(int sig) {
    terminate_flag = 1; // Устанавливаем флаг завершения
}

void semaphore_op(int sem_id, int sem_num, int op) {
    struct sembuf sem_op = {sem_num, op, 0};
    semop(sem_id, &sem_op, 1);
    // Проверяем флаг завершения после каждой операции семафора
    if (terminate_flag) {
        printf("Производитель %d завершает работу.\n", getpid());
        exit(0); // Завершаем процесс сразу
    }
}

unsigned short calculate_hash(Message* message) {
    unsigned short hash = 0;
    message->hash = 0;
    for (int i = 0; i < message->size; i++) {
        hash += (unsigned char)message->data[i];
    }
    return hash;
}

int main() {
    signal(SIGTERM, termination_handler); // Устанавливаем обработчик SIGTERM

    int shm_id = shmget(SHM_KEY, sizeof(MessageQueue), 0666);
    if (shm_id == -1) {
        perror("Ошибка подключения к общей памяти");
        exit(1);
    }

    MessageQueue* queue = (MessageQueue*)shmat(shm_id, NULL, 0);
    if (queue == (void*)-1) {
        perror("Ошибка подключения к очереди");
        exit(1);
    }

    int sem_id = semget(SEM_KEY, 3, 0666);
    if (sem_id == -1) {
        perror("Ошибка подключения к семафорам");
        exit(1);
    }

    srand(time(NULL) ^ getpid()); // Инициализация генератора случайных чисел

    printf("Производитель %d запущен.\n", getpid());

    while (1) {
        // Проверяем флаг завершения перед началом итерации
        if (terminate_flag) {
            printf("Производитель %d завершает работу.\n", getpid());
            break;
        }

        // Генерация сообщения
        Message message;
        message.type = 'A' + (rand() % 26);
        message.size = rand() % 256 + 1;
        for (int i = 0; i < message.size; i++) {
            message.data[i] = 'A' + (rand() % 26);
        }
        message.hash = calculate_hash(&message);

        // Ожидаем свободного места (с проверкой завершения после операции)
        semaphore_op(sem_id, 0, -1);
        semaphore_op(sem_id, 2, -1);

        // Добавление сообщения в очередь
        queue->buffer[queue->tail] = message;
        queue->tail = (queue->tail + 1) % QUEUE_SIZE;
        queue->free_slots--;
        queue->added_count++;

        printf("Производитель %d: добавлено сообщение (тип '%c', размер %d, hash %u). Всего добавлено: %d\n",
               getpid(), message.type, message.size, message.hash, queue->added_count);

        // Освобождаем мьютекс и увеличиваем занятые места (с проверкой завершения)
        semaphore_op(sem_id, 2, 1);
        semaphore_op(sem_id, 1, 1);

        // Задержка для наблюдения
        sleep(3);
    }

    return 0;
}
