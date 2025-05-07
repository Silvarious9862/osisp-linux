#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Структура индексной записи
struct index_s {
    double time_mark;   // временная метка
    uint64_t recno;     // номер записи в таблице
};

// Заголовок индексного файла
struct index_hdr_s {
    uint64_t records;   // количество записей
    struct index_s idx[]; // массив записей
};

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <имя_файла>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *filename = argv[1];

    // Открытие файла для чтения
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Ошибка открытия файла");
        return EXIT_FAILURE;
    }

    // Чтение количества записей
    uint64_t records;
    if (fread(&records, sizeof(uint64_t), 1, file) != 1) {
        fprintf(stderr, "Ошибка чтения заголовка файла\n");
        fclose(file);
        return EXIT_FAILURE;
    }

    printf("Количество записей: %lu\n", records);

    // Чтение и вывод записей
    struct index_s record;
    for (uint64_t i = 0; i < records; i++) {
        if (fread(&record, sizeof(struct index_s), 1, file) != 1) {
            fprintf(stderr, "Ошибка чтения записи %lu\n", i + 1);
            fclose(file);
            return EXIT_FAILURE;
        }

        //printf("Запись %lu:\n", i + 1);
        printf("---\t");
        printf("Запись %lu\n", record.recno);
        printf("Временная метка: %.10f\n", record.time_mark);
    }

    // Закрытие файла
    fclose(file);

    printf("Файл успешно прочитан и выведен.\n");
    return EXIT_SUCCESS;
}
