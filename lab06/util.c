#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

struct index_s {
    double time_mark;
    uint64_t recno;
};

struct index_hdr_s {
    uint64_t records;
    struct index_s idx[];
};

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <файл_данных>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        perror("Ошибка открытия файла");
        return EXIT_FAILURE;
    }

    uint64_t records;
    fread(&records, sizeof(uint64_t), 1, file);
    printf("Количество записей: %lu\n", records);

    struct index_s record;
    for (uint64_t i = 0; i < records; i++) {
        fread(&record, sizeof(struct index_s), 1, file);
        printf("Запись %lu: Временная метка = %.10f, Номер записи = %lu\n",
               i + 1, record.time_mark, record.recno);
    }

    fclose(file);
    return EXIT_SUCCESS;
}
