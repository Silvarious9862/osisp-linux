#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include "common.h"
#include "ipc.h"

volatile sig_atomic_t terminate_flag = 0;

void termination_handler(int sig) {
    terminate_flag = 1;
}

int main() {
    signal(SIGTERM, termination_handler);

    int shm_id = shmget(SHM_KEY, sizeof(MessageQueue), 0666);
    if (shm_id == -1) {
        perror("Ошибка подключения к общей памяти");
        exit(1);
    }

    MessageQueue* queue = (MessageQueue*) shmat(shm_id, NULL, 0);
    if (queue == (void*)-1) {
        perror("Ошибка подключения к очереди");
        exit(1);
    }

    int sem_id = semget(SEM_KEY, 3, 0666);
    if (sem_id == -1) {
        perror("Ошибка подключения к семафорам");
        exit(1);
    }

    srand(time(NULL) ^ getpid());
    printf("Производитель %d запущен.\n", getpid());

    while (1) {
        if (terminate_flag) {
            printf("Производитель %d завершает работу.\n", getpid());
            break;
        }

        Message message;
        message.type = 'A' + (rand() % 26);
        
        /* 
         * Генерация размера сообщения:
         * Используем rand() % 257. Если результат равен 0, генерируем заново.
         * Если результат равен 256, то в поле size записываем 0 (это специальное значение,
         * означающее, что реальная длина данных равна 256 байт).
         */
        int r = rand() % 257;
        while (r == 0) {
            r = rand() % 257;
        }
        
        int actual_len;  // реальная длина данных
        if (r == 256) {
            message.size = 0;  // особое значение, означающее 256 байт
            actual_len = 256;
        } else {
            message.size = r;
            actual_len = r;
        }
        
        /*
         * Вычисляем выровненную длину поля data:
         * Поскольку данные должны иметь длину, кратную 4м байтам, вычисляем:
         * padded_len = ((actual_len + 3) / 4) * 4.
         */
        int padded_len = ((actual_len + 3) / 4) * 4;
        
        /* Генерируем данные:
         * первые actual_len байт заполняем случайными буквами,
         * оставшиеся байты (до padded_len) заполняем нулём.
         */
        for (int i = 0; i < padded_len; i++) {
            if (i < actual_len) {
                message.data[i] = 'A' + (rand() % 26);
            } else {
                message.data[i] = 0;
            }
        }

        /* Вычисляем контрольную сумму.
         * Функция calculate_hash в данном случае суммирует ровно
         * actual_len байт (если message.size равен 0, считается 256).
         */
        message.hash = calculate_hash(&message);

        // Ожидание свободного места и захват мьютекса
        semaphore_op(sem_id, 0, -1);
        semaphore_op(sem_id, 2, -1);

        // Добавление сообщения в очередь
        queue->buffer[queue->tail] = message;
        queue->tail = (queue->tail + 1) % QUEUE_SIZE;
        queue->free_slots--;
        queue->added_count++;

        printf("Производитель %d: добавлено сообщение (тип '%c', ", getpid(), message.type);
        if (message.size == 0) {
            printf("размер %d, ", 256);
        } else {
            printf("размер %d, ", message.size);
        }
        printf("hash %u). Всего добавлено: %d\n", message.hash, queue->added_count);

        // Освобождение мьютекса и индикация заполненного слота
        semaphore_op(sem_id, 2, 1);
        semaphore_op(sem_id, 1, 1);

        sleep(3);
    }

    return 0;
}
