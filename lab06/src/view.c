#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include "index.h"  // путь к заголовкам, размещённым в ./lib/

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    FILE *fp = fopen(argv[1], "rb");
    if (!fp) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    // Чтение заголовка: количество записей (uint64_t)
    uint64_t num_records;
    if (fread(&num_records, sizeof(num_records), 1, fp) != 1) {
        fprintf(stderr, "Error reading header from file.\n");
        fclose(fp);
        return EXIT_FAILURE;
    }
    printf("Number of records: %" PRIu64 "\n", num_records);

    // Считываем и выводим все индексные записи
    struct index_s record;
    for (uint64_t i = 0; i < num_records; i++) {
        if (fread(&record, sizeof(record), 1, fp) != 1) {
            fprintf(stderr, "Error reading record %" PRIu64 "\n", i);
            fclose(fp);
            return EXIT_FAILURE;
        }
        printf("Record %" PRIu64 ": time_mark = %f, recno = %" PRIu64 "\n",
               i + 1,
               record.time_mark,
               record.recno);
    }
    
    fclose(fp);
    return EXIT_SUCCESS;
}
