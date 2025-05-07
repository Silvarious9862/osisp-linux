#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

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

// Функция для генерации случайной временной метки
double generate_time_mark() {
    int64_t julian_day = 15020 + rand() % (int64_t)(time(NULL) / 86400 - 15020); // диапазон модифицированных юлианских дат
    double fraction = (rand() % 86400) / 86400.0; // случайная дробная часть дня
    return julian_day + fraction;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Использование: %s <имя_файла> <количество_записей>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *filename = argv[1];
    uint64_t records = atoll(argv[2]);

    if (records % 256 != 0) {
        fprintf(stderr, "Количество записей должно быть кратным 256.\n");
        return EXIT_FAILURE;
    }

    // Открытие файла для записи
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Ошибка при открытии файла");
        return EXIT_FAILURE;
    }

    // Выделение памяти для заголовка и массива записей
    struct index_hdr_s *header = malloc(sizeof(uint64_t) + sizeof(struct index_s) * records);
    if (!header) {
        perror("Ошибка выделения памяти");
        fclose(file);
        return EXIT_FAILURE;
    }

    header->records = records;

    // Инициализация генератора случайных чисел
    srand(time(NULL));

    // Заполнение записей случайными данными
    for (uint64_t i = 0; i < records; i++) {
        header->idx[i].time_mark = generate_time_mark();
        header->idx[i].recno = i + 1; // последовательный номер (может быть заменен на случайный)
    }

    // Запись данных в файл
    fwrite(header, sizeof(uint64_t) + sizeof(struct index_s) * records, 1, file);

    // Очистка ресурсов
    free(header);
    fclose(file);

    printf("Файл успешно сгенерирован: %s\n", filename);
    return EXIT_SUCCESS;
}
