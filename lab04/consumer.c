#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define SHM_KEY 0x1234
#define SEM_KEY 0x5678
#define QUEUE_SIZE 10

// Структура сообщения
typedef struct {
    char type;                 // Тип сообщения
    unsigned short hash;       // Контрольная сумма
    unsigned char size;        // Размер данных
    char data[256];            // Данные сообщения
} Message;

// Структура очереди
typedef struct {
    Message buffer[QUEUE_SIZE]; // Кольцевой буфер
    int head;                   // Указатель на голову очереди
    int tail;                   // Указатель на хвост очереди
    int added_count;            // Счётчик добавленных сообщений
    int removed_count;          // Счётчик извлечённых сообщений
    int free_slots;             // Количество свободных мест
} MessageQueue;

volatile sig_atomic_t terminate_flag = 0; // Флаг завершения

// Обработчик сигнала SIGTERM
void termination_handler(int sig) {
    terminate_flag = 1; // Устанавливаем флаг завершения
}

// Утилита для работы с семафором
void semaphore_op(int sem_id, int sem_num, int op) {
    struct sembuf sem_op = {sem_num, op, 0};
    semop(sem_id, &sem_op, 1);
    // Проверяем флаг завершения после каждой операции семафора
    if (terminate_flag) {
        printf("Потребитель %d завершает работу.\n", getpid());
        exit(0); // Завершаем процесс
    }
}

// Функция для проверки контрольной суммы
int verify_hash(Message* message) {
    unsigned short expected_hash = 0;
    for (int i = 0; i < message->size; i++) {
        expected_hash += (unsigned char)message->data[i];
    }
    return expected_hash == message->hash;
}



int main() {
    // Установка обработчика SIGTERM
    signal(SIGTERM, termination_handler);

    // Подключение к общей памяти
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

    // Подключение к семафорам
    int sem_id = semget(SEM_KEY, 3, 0666);
    if (sem_id == -1) {
        perror("Ошибка подключения к семафорам");
        exit(1);
    }

    printf("Потребитель %d запущен.\n", getpid());

    while (1) {
        // Проверяем флаг завершения перед началом итерации
        if (terminate_flag) {
            printf("Потребитель %d завершает работу.\n", getpid());
            break;
        }

        // Ожидание заполненного слота (с проверкой завершения после операции)
        semaphore_op(sem_id, 1, -1);
        semaphore_op(sem_id, 2, -1); // Захват мьютекса

        // Извлечение сообщения из очереди
        Message message = queue->buffer[queue->head];
        queue->head = (queue->head + 1) % QUEUE_SIZE;
        queue->free_slots++;
        queue->removed_count++;

        // Вывод информации о сообщении и новом значении счётчика извлечённых сообщений
        printf("Потребитель %d: извлечено сообщение (тип '%c', размер %d, hash %u, данные: \"%.*s\"). Всего извлечено: %d\n",
               getpid(), message.type, message.size, message.hash, message.size, message.data, queue->removed_count);

        // Освобождение мьютекса и увеличение свободных слотов
        semaphore_op(sem_id, 2, 1);
        semaphore_op(sem_id, 0, 1);

        // Проверка контрольной суммы
        if (verify_hash(&message)) {
            printf("Потребитель %d: сообщение корректно.\n", getpid());
        } else {
            printf("Потребитель %d: сообщение повреждено!\n", getpid());
        }

        // Задержка для наблюдения
        sleep(3);
    }

    return 0;
}
