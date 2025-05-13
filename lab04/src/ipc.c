#include "ipc.h"
#include <sys/sem.h>
#include <unistd.h>

void semaphore_op(int sem_id, int sem_num, int op) {
    struct sembuf sem_op = {sem_num, op, 0};
    semop(sem_id, &sem_op, 1);
}

unsigned short calculate_hash(Message* message) {
    unsigned short hash = 0;
    int len = (message->size == 0 ? 256 : message->size);
    for (int i = 0; i < len; i++) {
        hash += (unsigned char) message->data[i];
    }
    return hash;
}

int verify_hash(Message* message) {
    unsigned short expected_hash = 0;
    int len = (message->size == 0 ? 256 : message->size);
    for (int i = 0; i < len; i++) {
        expected_hash += (unsigned char) message->data[i];
    }
    return expected_hash == message->hash;
}
