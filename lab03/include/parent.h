#ifndef PARENT_H
#define PARENT_H

#include <sys/types.h>

#define MAX_CHILDREN 100

// Массив для хранения PID дочерних процессов и счётчик
extern pid_t children[MAX_CHILDREN];
extern int child_count;

// Прототипы функций управления дочерними процессами
void add_child(void);
void remove_last_child(void);
void list_processes(void);
void kill_all_children(void);
void handle_input(void);

#endif /* PARENT_H */
