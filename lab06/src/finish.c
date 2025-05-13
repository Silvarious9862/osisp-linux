#include "finish.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

/* Разрушаем глобальный барьер, определённый в prep.c.
 * Обратите внимание: переменная prep_barrier объявлена в prep.h как extern.
 */
void finish_cleanup_prep(void) {
    if(prep_barrier_initialized) {
        int res = pthread_barrier_destroy(&prep_barrier);
        if (res != 0) {
            fprintf(stderr, "Ошибка разрушения prep_barrier: %s\n", strerror(res));
        } else {
            printf("prep_barrier успешно разрушен.\n");
        }
        prep_barrier_initialized = 0;
    } else {
        printf("prep_barrier не инициализирован или уже разрушен\n");
    }

}

/* Завершает работу этапа сортировки, разрушая барьер и мьютекс, созданные в sort_data. */
void finish_cleanup_sort(sort_data *sd) {
    if (pthread_barrier_destroy(&sd->barrier) != 0) {
        perror("Ошибка разрушения барьера в sort_data");
    } else {
        //printf("Барьер в sort_data успешно разрушен.\n");
    }
    if (pthread_mutex_destroy(&sd->mutex) != 0) {
        perror("Ошибка разрушения мьютекса в sort_data");
    } else {
        //printf("Мьютекс в sort_data успешно разрушен.\n");
    }
}

/* Завершает работу этапа слияния, разрушая барьер и мьютекс, созданные в merge_data. */
void finish_cleanup_merge(merge_data *md) {
    if (pthread_barrier_destroy(&md->barrier) != 0) {
        perror("Ошибка разрушения барьера в merge_data");
    } else {
        //printf("Барьер в merge_data успешно разрушен.\n");
    }
    if (pthread_mutex_destroy(&md->mutex) != 0) {
        perror("Ошибка разрушения мьютекса в merge_data");
    } else {
        //printf("Мьютекс в merge_data успешно разрушен.\n");
    }
}
