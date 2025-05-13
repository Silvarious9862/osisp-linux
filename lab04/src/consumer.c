#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <string.h>
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
    if (queue == (void*) -1) {
        perror("Ошибка подключения к очереди");
        exit(1);
    }

    int sem_id = semget(SEM_KEY, 3, 0666);
    if (sem_id == -1) {
        perror("Ошибка подключения к семафорам");
        exit(1);
    }

    printf("Потребитель %d запущен.\n", getpid());

    while (1) {
        if (terminate_flag) {
            printf("Потребитель %d завершает работу.\n", getpid());
            break;
        }

        semaphore_op(sem_id, 1, -1);
        semaphore_op(sem_id, 2, -1);

        Message message = queue->buffer[queue->head];
        queue->head = (queue->head + 1) % QUEUE_SIZE;
        queue->free_slots++;
        queue->removed_count++;

        printf("Потребитель %d: извлечено сообщение (тип '%c', размер %d, hash %u, данные: \"%.*s\"). Всего извлечено: %d\n",
               getpid(), message.type, message.size, message.hash, message.size, message.data, queue->removed_count);

        semaphore_op(sem_id, 2, 1);
        semaphore_op(sem_id, 0, 1);

        if (verify_hash(&message)) {
            printf("Потребитель %d: сообщение корректно.\n", getpid());
        } else {
            printf("Потребитель %d: сообщение повреждено!\n", getpid());
        }

        sleep(3);
    }

    return 0;
}
