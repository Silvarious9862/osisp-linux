#ifndef RESULT_H
#define RESULT_H
#include <limits.h>

typedef struct file_entry {
    char full_path[PATH_MAX];
    off_t file_size;
} file_entry;

extern file_entry *file_list;
extern size_t file_count;
extern size_t file_list_capacity;
extern int recursive_flag;
extern int time_flag;

void print_filtered_file_list(void);

void print_summary(void);

void print_size_listing(void);

void print_size_time_listing(void);

#endif  // RESULT_H
