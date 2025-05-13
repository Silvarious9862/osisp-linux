#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>

// Структура одной индексной записи
struct index_s {
    double time_mark; // временная метка (модифицированная юлианская дата)
    uint64_t recno;   // номер записи в таблице БД
};

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <num_records> <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *endptr;
    uint64_t num_records = strtoull(argv[1], &endptr, 10);
    if (*endptr != '\0' || num_records == 0) {
        fprintf(stderr, "Invalid number of records.\n");
        return EXIT_FAILURE;
    }
    if (num_records % 256 != 0) {
        fprintf(stderr, "Error: number of records (%llu) must be divisible by 256.\n",
                (unsigned long long)num_records);
        return EXIT_FAILURE;
    }

    FILE *fp = fopen(argv[2], "wb");
    if (!fp) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    // Записываем заголовок: количество записей (uint64_t)
    if (fwrite(&num_records, sizeof(num_records), 1, fp) != 1) {
        perror("fwrite header");
        fclose(fp);
        return EXIT_FAILURE;
    }

    // Выделяем память для массива индексных записей
    struct index_s *records_array = malloc(num_records * sizeof(struct index_s));
    if (!records_array) {
        perror("malloc");
        fclose(fp);
        return EXIT_FAILURE;
    }

    // Инициализация генератора случайных чисел
    srandom((unsigned int) time(NULL));

    // Вычисляем максимальное значение для целой части временной метки.
    // Модифицированная юлианская дата для 1970-01-01 равна 40587.
    // Вычисляем MJD для "вчера": MJD = floor((time(NULL)-86400)/86400) + 40587.
    time_t now = time(NULL);
    time_t yesterday = now - 86400;
    unsigned long days_since_epoch = yesterday / 86400;
    unsigned long max_int = days_since_epoch + 40587;
    const unsigned long min_int = 15020;
    if (max_int < min_int) {
        fprintf(stderr, "Error: computed max_int (%lu) is less than min_int (%lu).\n", max_int, min_int);
        free(records_array);
        fclose(fp);
        return EXIT_FAILURE;
    }
    unsigned long range = max_int - min_int + 1;

    // Генерируем каждую индексную запись
    for (uint64_t i = 0; i < num_records; i++) {
        unsigned long int_part = (unsigned long)(random() % range) + min_int;
        // Генерируем дробную часть, приводя результат к диапазону [0, 1)
        double frac = (double) random() / ((double) RAND_MAX + 1.0);
        records_array[i].time_mark = (double) int_part + frac;
        records_array[i].recno = i + 1;
    }

    // Записываем массив индексных записей в файл
    size_t written = fwrite(records_array, sizeof(struct index_s), num_records, fp);
    if (written != num_records) {
        fprintf(stderr, "Error writing records: wrote %zu of %llu records\n",
                written, (unsigned long long)num_records);
        free(records_array);
        fclose(fp);
        return EXIT_FAILURE;
    }

    free(records_array);
    fclose(fp);
    return EXIT_SUCCESS;
}
