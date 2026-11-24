#include <stdio.h>
#include <stdlib.h>
#include "mem.h"

MemBlock *mem_init(size_t total_size) {
    if (total_size == 0) return NULL;
    MemBlock *head = malloc(sizeof(MemBlock));
    if (head == NULL) return NULL;
    head->pid = -1;
    head->start = 0;
    head->size = total_size;
    head->free = true;
    head->next = NULL;
    return head;
}

bool mem_alloc(MemBlock *head, int pid, size_t size) {
    if (head == NULL || pid < 0 || size == 0) return false;
    for (MemBlock *cur = head; cur != NULL; cur = cur->next) {
        if (cur->free && cur->size >= size) {
            if (cur->size > size) {
                MemBlock *rem = malloc(sizeof(MemBlock));
                if (rem == NULL) return false;
                rem->pid = -1;
                rem->start = cur->start + size;
                rem->size = cur->size - size;
                rem->free = true;
                rem->next = cur->next;
                cur->size = size;
                cur->next = rem;
            }
            cur->pid = pid;
            cur->free = false;
            return true;
        }
    }
    return false;
}

void mem_free(MemBlock *head, int pid) {
    if (head == NULL || pid < 0) return;
    for (MemBlock *cur = head; cur != NULL; cur = cur->next) {
        if (!cur->free && cur->pid == pid) {
            cur->free = true;
            cur->pid = -1;
        }
    }
    MemBlock *cur = head;
    while (cur != NULL && cur->next != NULL) {
        if (cur->free && cur->next->free) {
            MemBlock *tmp = cur->next;
            cur->size += tmp->size;
            cur->next = tmp->next;
            free(tmp);
        } else {
            cur = cur->next;
        }
    }
}

void mem_print(const MemBlock *head) {
    printf("=== ETAT MEMOIRE ===\n");
    for (const MemBlock *cur = head; cur != NULL; cur = cur->next) {
        if (cur->free) {
            printf("[0x%zx..0x%zx] taille=%zu LIBRE\n", cur->start, cur->start + cur->size, cur->size);
        } else {
            printf("[0x%zx..0x%zx] taille=%zu PID=%d\n", cur->start, cur->start + cur->size, cur->size, cur->pid);
        }
    }
}

void mem_destroy(MemBlock *head) {
    MemBlock *cur = head;
    while (cur != NULL) {
        MemBlock *next = cur->next;
        free(cur);
        cur = next;
    }
}
