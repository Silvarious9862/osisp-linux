// main.c - Новая архитектура (потоковая модель с динамическим изменением очереди)
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "common.h"         // Определяет структуру Message и общие константы
#include "thread_queue.h"   // Определяет ThreadMessageQueue и операции с динамической очередью

#define MAX_THREADS 100

volatile int terminate_flag = 0;   // Флаг завершения работы системы
ThreadMessageQueue queue;          // Глобальная динамическая очередь

// Функция вычисления контрольной суммы сообщения.
// Если поле size равно 0, считается, что реальная длина данных равна 256 байт.
unsigned short calculate_hash(Message *message) {
    unsigned short hash = 0;
    int len = (message->size == 0 ? 256 : message->size);
    for (int i = 0; i < len; i++) {
        hash += (unsigned char) message->data[i];
    }
    return hash;
}

// Функция потока-производителя
void *producer_thread(void *arg) {
    int id = *(int*)arg;
    free(arg);
    while (!terminate_flag) {
        Message message;
        // Формирование типа сообщения
        message.type = 'A' + (rand() % 26);
        
        // Генерация размера: значение = rand() % 257, повторять, если равно 0.
        int r = rand() % 257;
        while (r == 0) {
            r = rand() % 257;
        }
        int actual_len = (r == 256) ? 256 : r;
        // Если r == 256, то записываем 0 в поле size (означает, что реальная длина — 256)
        message.size = (r == 256) ? 0 : r;
        
        // Выравнивание длины данных до ближайшего числа, кратного 4:
        int padded_len = ((actual_len + 3) / 4) * 4;
        for (int i = 0; i < padded_len; i++) {
            if (i < actual_len)
                message.data[i] = 'A' + (rand() % 26);
            else
                message.data[i] = 0;
        }
        // Вычисление контрольной суммы (hash) сообщения
        message.hash = calculate_hash(&message);
        
        // Ожидание появления свободного слота
        sem_wait(&queue.sem_free);
        pthread_mutex_lock(&queue.mutex);
        
        // Добавление сообщения в очередь
        queue.buffer[queue.tail] = message;
        queue.tail = (queue.tail + 1) % queue.capacity;
        queue.count++;
        queue.added_count++;
        printf("Producer[%d]: добавлено сообщение (тип '%c', размер %d, hash %u). Всего добавлено: %d\n",
               id, message.type, (message.size == 0 ? 256 : message.size), message.hash, queue.added_count);
        
        pthread_mutex_unlock(&queue.mutex);
        sem_post(&queue.sem_full);
        
        // Задержка для наблюдения
        sleep(3);
    }
    printf("Producer[%d] завершает работу\n", id);
    return NULL;
}

// Функция потока-потребителя
void *consumer_thread(void *arg) {
    int id = *(int*)arg;
    free(arg);
    while (!terminate_flag) {
        // Ожидание появления заполненного слота
        sem_wait(&queue.sem_full);
        pthread_mutex_lock(&queue.mutex);
        
        // Извлечение сообщения из очереди
        Message message = queue.buffer[queue.head];
        queue.head = (queue.head + 1) % queue.capacity;
        queue.count--;
        queue.removed_count++;
        
        pthread_mutex_unlock(&queue.mutex);
        sem_post(&queue.sem_free);
        
        printf("Consumer[%d]: извлечено сообщение (тип '%c', размер %d, hash %u). Всего извлечено: %d\n",
               id, message.type, (message.size == 0 ? 256 : message.size), message.hash, queue.removed_count);
        if (calculate_hash(&message) == message.hash)
            printf("Consumer[%d]: сообщение корректно.\n", id);
        else
            printf("Consumer[%d]: сообщение повреждено!\n", id);
        
        sleep(3);
    }
    printf("Consumer[%d] завершает работу\n", id);
    return NULL;
}

// Функция вывода текущего состояния очереди
void print_status() {
    pthread_mutex_lock(&queue.mutex);
    printf("\n--- Состояние очереди ---\n");
    printf("Ёмкость очереди: %d\n", queue.capacity);
    printf("Элементов в очереди: %d\n", queue.count);
    printf("Свободных слотов: %d\n", queue.capacity - queue.count);
    printf("Добавлено сообщений: %d\n", queue.added_count);
    printf("Извлечено сообщений: %d\n", queue.removed_count);
    pthread_mutex_unlock(&queue.mutex);
}

int main() {
    srand(time(NULL));
    // Инициализация динамической очереди с начальнoй ёмкостью 10
    if (init_thread_queue(&queue, 10) != 0) {
        perror("Ошибка инициализации очереди");
        return 1;
    }
    
    pthread_t producers[MAX_THREADS], consumers[MAX_THREADS];
    int prod_count = 0, cons_count = 0;
    char command[10];
    
    printf("Команды:\n");
    printf("  + : добавить поток-производитель\n");
    printf("  - : добавить поток-потребитель\n");
    printf("  > : увеличить размер очереди\n");
    printf("  < : уменьшить размер очереди\n");
    printf("  p : вывести состояние очереди\n");
    printf("  q : завершение работы\n");
    
    // Основной цикл обработки команд пользователя
    while (1) {
        if (fgets(command, sizeof(command), stdin) != NULL) {
            if (command[0] == 'q') {
                terminate_flag = 1;
                break;
            } else if (command[0] == '+') {
                int *id = malloc(sizeof(int));
                if (id == NULL) continue;
                *id = prod_count + 1;
                if (pthread_create(&producers[prod_count], NULL, producer_thread, id) != 0) {
                    perror("Ошибка создания потока-производителя");
                    free(id);
                } else {
                    prod_count++;
                }
            } else if (command[0] == '-') {
                int *id = malloc(sizeof(int));
                if (id == NULL) continue;
                *id = cons_count + 1;
                if (pthread_create(&consumers[cons_count], NULL, consumer_thread, id) != 0) {
                    perror("Ошибка создания потока-потребителя");
                    free(id);
                } else {
                    cons_count++;
                }
            } else if (command[0] == 'p') {
                print_status();
            } else if (command[0] == '>') {
                int new_capacity;
                pthread_mutex_lock(&queue.mutex);
                new_capacity = queue.capacity + 1;
                pthread_mutex_unlock(&queue.mutex);
                if (resize_thread_queue(&queue, new_capacity) == 0)
                    printf("Очередь увеличена до %d слотов\n", new_capacity);
                else
                    printf("Ошибка при увеличении очереди\n");
            } else if (command[0] == '<') {
                int new_capacity;
                pthread_mutex_lock(&queue.mutex);
                new_capacity = queue.capacity - 1;
                if (new_capacity < queue.count) {
                    printf("Невозможно уменьшить очередь ниже количества элементов (%d)\n", queue.count);
                    pthread_mutex_unlock(&queue.mutex);
                    continue;
                }
                pthread_mutex_unlock(&queue.mutex);
                if (resize_thread_queue(&queue, new_capacity) == 0)
                    printf("Очередь уменьшена до %d слотов\n", new_capacity);
                else
                    printf("Ошибка при уменьшении очереди\n");
            } else {
                printf("Неизвестная команда: %s", command);
            }
        }
    }
    
    // Ожидание завершения всех созданных потоков
    for (int i = 0; i < prod_count; i++) {
        pthread_join(producers[i], NULL);
    }
    for (int i = 0; i < cons_count; i++) {
        pthread_join(consumers[i], NULL);
    }
    
    destroy_thread_queue(&queue);
    printf("Программа завершена.\n");
    return 0;
}
