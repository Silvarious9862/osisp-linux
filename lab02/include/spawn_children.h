#ifndef SPAWN_CHILDREN_H
#define SPAWN_CHILDREN_H

void spawn_child_plus(int child_number, const char *env_file);
void spawn_child_star(int child_number);
void spawn_child_amp(int child_number);

#endif // SPAWN_CHILDREN_H
