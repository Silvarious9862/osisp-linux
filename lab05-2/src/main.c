#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "common.h"
#include "queue.h"
#include "producer_consumer.h"

// Глобальные переменные
volatile int terminate_flag = 0;
MessageQueue queue;

#define MAX_THREADS 100

int main() {
    srand(time(NULL));
    if (init_queue(&queue, INITIAL_CAPACITY) != 0) {
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
    printf("  < : уменьшить размер очереди (только когда очередь пуста)\n");
    printf("  p : вывести состояние очереди\n");
    printf("  q : завершение работы\n");
    
    while (fgets(command, sizeof(command), stdin) != NULL) {
        if (command[0] == 'q') {
            terminate_flag = 1;
            pthread_mutex_lock(&queue.mutex);
            pthread_cond_broadcast(&queue.not_empty);
            pthread_cond_broadcast(&queue.not_full);
            pthread_mutex_unlock(&queue.mutex);
            break;
        } else if (command[0] == '+') {
            int *id = malloc(sizeof(int));
            if (!id) continue;
            *id = prod_count + 1;
            if (pthread_create(&producers[prod_count], NULL, producer_thread, id) != 0) {
                perror("Ошибка создания производителя");
                free(id);
            } else {
                prod_count++;
            }
        } else if (command[0] == '-') {
            int *id = malloc(sizeof(int));
            if (!id) continue;
            *id = cons_count + 1;
            if (pthread_create(&consumers[cons_count], NULL, consumer_thread, id) != 0) {
                perror("Ошибка создания потребителя");
                free(id);
            } else {
                cons_count++;
            }
        } else if (command[0] == 'p') {
            print_status(&queue);
        } else if (command[0] == '>') {
            int new_capacity;
            pthread_mutex_lock(&queue.mutex);
            new_capacity = queue.capacity + 1;
            pthread_mutex_unlock(&queue.mutex);
            if (resize_queue(&queue, new_capacity) == 0)
                printf("Очередь увеличена до %d слотов\n", new_capacity);
            else
                printf("Ошибка при увеличении очереди\n");
        } else if (command[0] == '<') {
            pthread_mutex_lock(&queue.mutex);
            if (queue.count != 0) {
                printf("Нельзя уменьшить очередь, пока она не пуста (занято %d элементов)\n", queue.count);
                pthread_mutex_unlock(&queue.mutex);
                continue;
            }
            if (queue.capacity <= 2) {
                printf("Минимальная ёмкость очереди равна 2. Уменьшение невозможно.\n");
                pthread_mutex_unlock(&queue.mutex);
                continue;
            }
            int new_capacity = queue.capacity - 1;
            pthread_mutex_unlock(&queue.mutex);
            if (resize_queue(&queue, new_capacity) == 0)
                printf("Очередь уменьшена до %d слотов\n", new_capacity);
            else
                printf("Ошибка при уменьшении очереди\n");
        } else {
            printf("Неизвестная команда: %s", command);
        }
    }
    
    // Рассылаем сигнал всем ожидающим потокам для завершения работы
    pthread_mutex_lock(&queue.mutex);
    pthread_cond_broadcast(&queue.not_empty);
    pthread_cond_broadcast(&queue.not_full);
    pthread_mutex_unlock(&queue.mutex);
    
    for (int i = 0; i < prod_count; i++) {
        pthread_join(producers[i], NULL);
    }
    for (int i = 0; i < cons_count; i++) {
        pthread_join(consumers[i], NULL);
    }
    
    destroy_queue(&queue);
    printf("Программа завершена.\n");
    return 0;
}
