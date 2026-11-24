#ifndef ESINOS_MEM_H
#define ESINOS_MEM_H
#include <stdbool.h>
#include <stddef.h>
#include "process.h"
#define ESINOS_MEM_SIZE 1024
typedef struct MemBlock {
    int pid;
    size_t start;
    size_t size;
    bool free;
    struct MemBlock *next;
} MemBlock;
MemBlock *mem_init(size_t total_size);
bool mem_alloc(MemBlock *head, int pid, size_t size);
void mem_free(MemBlock *head, int pid);
void mem_print(const MemBlock *head);
void mem_destroy(MemBlock *head);
#endif
