@'
#ifndef ESINOS_MEM_H
#define ESINOS_MEM_H

#include <stdbool.h>
#include <stddef.h>
#include "process.h"

#define ESINOS_MEM_SIZE 1024
#define MEM_MAX_FRAMES 64
#define MEM_MAX_REFS 128
#define MEM_TLB_SIZE 16

typedef struct {
    int page;
    int frame;
    bool valid;
    int last_used;
    int load_order;
} MemTlbEntry;

typedef struct {
    int page_size;
    int num_frames;
    int page_table[MEM_MAX_FRAMES];
    int frame_page[MEM_MAX_FRAMES];
    int frame_load_order[MEM_MAX_FRAMES];
    int frame_last_used[MEM_MAX_FRAMES];
    int next_order;
    int time_tick;
    int page_faults;
    MemTlbEntry tlb[MEM_TLB_SIZE];
    int tlb_size;
    int tlb_hits;
    int tlb_misses;
    double tlb_access_ns;
    double mem_access_ns;
} MemPaginationCtx;

/* Point d'entrée appelé par le menu principal (choix 2) */
void menu_memoire(void);

/* Pagination : traduction d'adresse, remplacement FIFO/LRU/Optimal, TLB et EAT */
void mem_pagination_init(MemPaginationCtx *ctx, int page_size, int num_frames,
                         int tlb_entries, double tlb_access_ns, double mem_access_ns);
bool mem_translate_address(MemPaginationCtx *ctx, int logical_addr,
                           int *out_page, int *out_offset, int *out_frame, int *out_phys_addr,
                           bool *out_tlb_hit, bool *out_page_fault);
double mem_compute_eat(const MemPaginationCtx *ctx);
void mem_sim_fifo(const int *refs, int n_refs, int num_frames, bool verbose);
void mem_sim_lru(const int *refs, int n_refs, int num_frames, bool verbose);
int mem_sim_optimal(const int *refs, int n_refs, int num_frames, bool verbose);

/* Bonus : allocateur contigu First-Fit */
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

#endif // esinos_mem_h
