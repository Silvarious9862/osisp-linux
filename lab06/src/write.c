#include "write.h"
#include <sys/mman.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

int write_sorted_result(scan_data *data) {
    if (!data || !data->map_ptr) {
        fprintf(stderr, "Ошибка: неверные данные отображения в write_sorted_result.\n");
        return -1;
    }

    /* Синхронизуем изменения в отображённой области с файлом */
    if (msync(data->map_ptr, data->memsize, MS_SYNC) < 0) {
        fprintf(stderr, "Ошибка синхронизации памяти с файлом: %s\n", strerror(errno));
        return -1;
    }
    printf("Отсортированный результат успешно записан в файл.\n");
    return 0;
}
