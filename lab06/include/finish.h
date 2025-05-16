#ifndef FINISH_H
#define FINISH_H

#include "prep.h"   // Чтобы получить extern-переменную prep_barrier.
#include "sort.h"   // Определение структуры sort_data.
#include "merge.h"  // Определение структуры merge_data.

extern int prep_barrier_initialized;

/**
 * finish_cleanup_prep() — разрушает глобальный барьер, созданный в модуле prep.
 */
void finish_cleanup_prep(void);

/**
 * finish_cleanup_sort() — разрушает барьер и мьютекс, используемые в
 * структуре сортировки блоков (sort_data).
 *
 * @sd: указатель на структуру sort_data, ранее инициализированную функцией sort_init().
 */
void finish_cleanup_sort(sort_data *sd);

/**
 * finish_cleanup_merge() — разрушает барьер и мьютекс, используемые в
 * структуре слияния блоков (merge_data).
 *
 * @md: указатель на структуру merge_data, ранее инициализированную функцией merge_init().
 */
void finish_cleanup_merge(merge_data *md);

#endif // FINISH_H
