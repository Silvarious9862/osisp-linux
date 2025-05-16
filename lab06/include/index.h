#ifndef INDEX_H
#define INDEX_H

#include <stdint.h>

struct index_s {
    double time_mark; // временная метка (модифицированная юлианская дата)
    uint64_t recno;   // номер записи в таблице БД
};

#endif // INDEX_H
