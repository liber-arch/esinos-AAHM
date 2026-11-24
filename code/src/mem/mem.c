#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mem.h"

static void vider_ligne(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
    }
}

static int lire_int(const char *prompt) {
    int val = 0;
    while (1) {
        printf("%s", prompt);
        if (scanf("%d", &val) == 1) {
            vider_ligne();
            return val;
        }
        vider_ligne();
        printf("Entree invalide, reessayez.\n");
    }
}

static double lire_double(const char *prompt) {
    double val = 0.0;
    while (1) {
        printf("%s", prompt);
        if (scanf("%lf", &val) == 1) {
            vider_ligne();
            return val;
        }
        vider_ligne();
        printf("Entree invalide, reessayez.\n");
    }
}

void mem_pagination_init(MemPaginationCtx *ctx, int page_size, int num_frames,
                         int tlb_entries, double tlb_access_ns, double mem_access_ns) {
    if (!ctx) return;
    memset(ctx, 0, sizeof(*ctx));
    ctx->page_size = (page_size > 0) ? page_size : 64;
    if (num_frames < 1) num_frames = 1;
    if (num_frames > MEM_MAX_FRAMES) num_frames = MEM_MAX_FRAMES;
    ctx->num_frames = num_frames;

    if (tlb_entries < 1) tlb_entries = 1;
    if (tlb_entries > MEM_TLB_SIZE) tlb_entries = MEM_TLB_SIZE;
    ctx->tlb_size = tlb_entries;

    ctx->tlb_access_ns = (tlb_access_ns >= 0.0) ? tlb_access_ns : 10.0;
    ctx->mem_access_ns = (mem_access_ns > 0.0) ? mem_access_ns : 100.0;

    for (int i = 0; i < MEM_MAX_FRAMES; i++) {
        ctx->page_table[i] = -1;
        ctx->frame_page[i] = -1;
        ctx->frame_load_order[i] = -1;
        ctx->frame_last_used[i] = -1;
    }
    for (int i = 0; i < MEM_TLB_SIZE; i++) {
        ctx->tlb[i].page = -1;
        ctx->tlb[i].frame = -1;
        ctx->tlb[i].valid = false;
        ctx->tlb[i].last_used = -1;
        ctx->tlb[i].load_order = -1;
    }
}

static int tlb_lookup(MemPaginationCtx *ctx, int page, int tick) {
    for (int i = 0; i < ctx->tlb_size; i++) {
        if (ctx->tlb[i].valid && ctx->tlb[i].page == page) {
            ctx->tlb[i].last_used = tick;
            return ctx->tlb[i].frame;
        }
    }
    return -1;
}

static void tlb_insert(MemPaginationCtx *ctx, int page, int frame, int tick, int order) {
    for (int i = 0; i < ctx->tlb_size; i++) {
        if (ctx->tlb[i].valid && ctx->tlb[i].page == page) {
            ctx->tlb[i].frame = frame;
            ctx->tlb[i].last_used = tick;
            return;
        }
    }
    int target = -1;
    for (int i = 0; i < ctx->tlb_size; i++) {
        if (!ctx->tlb[i].valid) {
            target = i;
            break;
        }
    }
    if (target == -1) {
        target = 0;
        for (int i = 1; i < ctx->tlb_size; i++) {
            if (ctx->tlb[i].last_used < ctx->tlb[target].last_used) {
                target = i;
            }
        }
    }
    ctx->tlb[target].page = page;
    ctx->tlb[target].frame = frame;
    ctx->tlb[target].valid = true;
    ctx->tlb[target].last_used = tick;
    ctx->tlb[target].load_order = order;
}

static void tlb_invalidate_page(MemPaginationCtx *ctx, int page) {
    for (int i = 0; i < ctx->tlb_size; i++) {
        if (ctx->tlb[i].valid && ctx->tlb[i].page == page) {
            ctx->tlb[i].valid = false;
        }
    }
}

bool mem_translate_address(MemPaginationCtx *ctx, int logical_addr,
                           int *out_page, int *out_offset, int *out_frame, int *out_phys_addr,
                           bool *out_tlb_hit, bool *out_page_fault) {
    if (!ctx || logical_addr < 0 || ctx->page_size <= 0) return false;

    int page = logical_addr / ctx->page_size;
    int offset = logical_addr % ctx->page_size;
    ctx->time_tick++;
    int tick = ctx->time_tick;

    bool hit = false;
    bool fault = false;
    int frame = tlb_lookup(ctx, page, tick);

    if (frame != -1) {
        hit = true;
        ctx->tlb_hits++;
        if (page < MEM_MAX_FRAMES && ctx->page_table[page] == frame) {
            ctx->frame_last_used[frame] = tick;
        }
    } else {
        ctx->tlb_misses++;
        if (page < MEM_MAX_FRAMES && ctx->page_table[page] != -1) {
            frame = ctx->page_table[page];
            ctx->frame_last_used[frame] = tick;
            tlb_insert(ctx, page, frame, tick, ctx->next_order++);
        } else {
            fault = true;
            ctx->page_faults++;
            int free_frame = -1;
            for (int f = 0; f < ctx->num_frames; f++) {
                if (ctx->frame_page[f] == -1) {
                    free_frame = f;
                    break;
                }
            }
            if (free_frame == -1) {
                free_frame = 0;
                for (int f = 1; f < ctx->num_frames; f++) {
                    if (ctx->frame_load_order[f] < ctx->frame_load_order[free_frame]) {
                        free_frame = f;
                    }
                }
                int old_page = ctx->frame_page[free_frame];
                if (old_page >= 0 && old_page < MEM_MAX_FRAMES) {
                    ctx->page_table[old_page] = -1;
                }
                tlb_invalidate_page(ctx, old_page);
            }
            frame = free_frame;
            ctx->frame_page[frame] = page;
            ctx->frame_load_order[frame] = ctx->next_order++;
            ctx->frame_last_used[frame] = tick;
            if (page < MEM_MAX_FRAMES) {
                ctx->page_table[page] = frame;
            }
            tlb_insert(ctx, page, frame, tick, ctx->frame_load_order[frame]);
        }
    }

    if (out_page) *out_page = page;
    if (out_offset) *out_offset = offset;
    if (out_frame) *out_frame = frame;
    if (out_phys_addr) *out_phys_addr = frame * ctx->page_size + offset;
    if (out_tlb_hit) *out_tlb_hit = hit;
    if (out_page_fault) *out_page_fault = fault;
    return true;
}

double mem_compute_eat(const MemPaginationCtx *ctx) {
    if (!ctx) return 0.0;
    int total = ctx->tlb_hits + ctx->tlb_misses;
    double hit_ratio = (total > 0) ? ((double)ctx->tlb_hits / (double)total) : 0.0;
    double miss_ratio = 1.0 - hit_ratio;
    /* Sans defaut de page : hit -> tlb + mem ; miss -> tlb + 2 * mem */
    return ctx->tlb_access_ns + hit_ratio * ctx->mem_access_ns +
           miss_ratio * (2.0 * ctx->mem_access_ns);
}

static void print_frames(const int *frames, int num_frames) {
    printf("[");
    for (int i = 0; i < num_frames; i++) {
        if (frames[i] < 0) {
            printf(" -");
        } else {
            printf(" %d", frames[i]);
        }
        if (i < num_frames - 1) printf(" |");
    }
    printf(" ]");
}

void mem_sim_fifo(const int *refs, int n_refs, int num_frames, bool verbose) {
    if (!refs || n_refs <= 0 || num_frames <= 0) return;
    if (num_frames > MEM_MAX_FRAMES) num_frames = MEM_MAX_FRAMES;

    int frames[MEM_MAX_FRAMES];
    int ages[MEM_MAX_FRAMES];
    int clock = 0;
    int faults = 0;

    for (int i = 0; i < num_frames; i++) {
        frames[i] = -1;
        ages[i] = 0;
    }

    if (verbose) printf("\n--- Simulation FIFO (%d cadres) ---\n", num_frames);
    for (int i = 0; i < n_refs; i++) {
        int p = refs[i];
        bool hit = false;
        for (int f = 0; f < num_frames; f++) {
            if (frames[f] == p) {
                hit = true;
                break;
            }
        }
        if (!hit) {
            faults++;
            int slot = -1;
            for (int f = 0; f < num_frames; f++) {
                if (frames[f] == -1) {
                    slot = f;
                    break;
                }
            }
            if (slot == -1) {
                slot = 0;
                for (int f = 1; f < num_frames; f++) {
                    if (ages[f] < ages[slot]) {
                        slot = f;
                    }
                }
            }
            frames[slot] = p;
            ages[slot] = clock++;
        }
        if (verbose) {
            printf("Etape %2d | ref=%2d | ", i + 1, p);
            print_frames(frames, num_frames);
            printf(" | %s\n", hit ? "HIT" : "DEFAUT");
        }
    }
    printf("Total defauts de page (FIFO) : %d / %d\n", faults, n_refs);
}

void mem_sim_lru(const int *refs, int n_refs, int num_frames, bool verbose) {
    if (!refs || n_refs <= 0 || num_frames <= 0) return;
    if (num_frames > MEM_MAX_FRAMES) num_frames = MEM_MAX_FRAMES;

    int frames[MEM_MAX_FRAMES];
    int last_used[MEM_MAX_FRAMES];
    int faults = 0;

    for (int i = 0; i < num_frames; i++) {
        frames[i] = -1;
        last_used[i] = -1;
    }

    if (verbose) printf("\n--- Simulation LRU (%d cadres) ---\n", num_frames);
    for (int i = 0; i < n_refs; i++) {
        int p = refs[i];
        bool hit = false;
        for (int f = 0; f < num_frames; f++) {
            if (frames[f] == p) {
                hit = true;
                last_used[f] = i;
                break;
            }
        }
        if (!hit) {
            faults++;
            int slot = -1;
            for (int f = 0; f < num_frames; f++) {
                if (frames[f] == -1) {
                    slot = f;
                    break;
                }
            }
            if (slot == -1) {
                slot = 0;
                for (int f = 1; f < num_frames; f++) {
                    if (last_used[f] < last_used[slot]) {
                        slot = f;
                    }
                }
            }
            frames[slot] = p;
            last_used[slot] = i;
        }
        if (verbose) {
            printf("Etape %2d | ref=%2d | ", i + 1, p);
            print_frames(frames, num_frames);
            printf(" | %s\n", hit ? "HIT" : "DEFAUT");
        }
    }
    printf("Total defauts de page (LRU) : %d / %d\n", faults, n_refs);
}

int mem_sim_optimal(const int *refs, int n_refs, int num_frames, bool verbose) {
    if (!refs || n_refs <= 0 || num_frames <= 0) return 0;
    if (num_frames > MEM_MAX_FRAMES) num_frames = MEM_MAX_FRAMES;

    int frames[MEM_MAX_FRAMES];
    int faults = 0;

    for (int i = 0; i < num_frames; i++) {
        frames[i] = -1;
    }

    if (verbose) printf("\n--- Simulation Optimal (%d cadres) ---\n", num_frames);
    for (int i = 0; i < n_refs; i++) {
        int p = refs[i];
        bool hit = false;
        for (int f = 0; f < num_frames; f++) {
            if (frames[f] == p) {
                hit = true;
                break;
            }
        }
        if (!hit) {
            faults++;
            int slot = -1;
            for (int f = 0; f < num_frames; f++) {
                if (frames[f] == -1) {
                    slot = f;
                    break;
                }
            }
            if (slot == -1) {
                int farthest = -1;
                slot = 0;
                for (int f = 0; f < num_frames; f++) {
                    int next_use = n_refs + 1;
                    for (int k = i + 1; k < n_refs; k++) {
                        if (refs[k] == frames[f]) {
                            next_use = k;
                            break;
                        }
                    }
                    if (next_use > farthest) {
                        farthest = next_use;
                        slot = f;
                    }
                }
            }
            frames[slot] = p;
        }
        if (verbose) {
            printf("Etape %2d | ref=%2d | ", i + 1, p);
            print_frames(frames, num_frames);
            printf(" | %s\n", hit ? "HIT" : "DEFAUT");
        }
    }
    printf("Total defauts de page (Optimal) : %d / %d\n", faults, n_refs);
    return faults;
}

static void lancer_demo_fifo_lru(void) {
    const int demo_refs[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2};
    int n = (int)(sizeof(demo_refs) / sizeof(demo_refs[0]));
    int cadres = 3;
    printf("\n=== DEMO PAGINATION : FIFO PUIS LRU (3 cadres) ===\n");
    mem_sim_fifo(demo_refs, n, cadres, true);
    mem_sim_lru(demo_refs, n, cadres, true);
}

static void saisir_et_simuler_remplacement(int mode) {
    int n = lire_int("Nombre de references (1..128) : ");
    if (n < 1) n = 1;
    if (n > MEM_MAX_REFS) n = MEM_MAX_REFS;

    int refs[MEM_MAX_REFS];
    for (int i = 0; i < n; i++) {
        char buf[64];
        snprintf(buf, sizeof(buf), "Reference [%d/%d] (numero de page) : ", i + 1, n);
        refs[i] = lire_int(buf);
    }
    int cadres = lire_int("Nombre de cadres (1..64) : ");
    if (mode == 1) mem_sim_fifo(refs, n, cadres, true);
    else if (mode == 2) mem_sim_lru(refs, n, cadres, true);
    else mem_sim_optimal(refs, n, cadres, true);
}

void menu_memoire(void) {
    MemPaginationCtx ctx;
    mem_pagination_init(&ctx, 64, 4, 4, 10.0, 100.0);

    int choix = -1;
    while (choix != 0) {
        printf("\n================ MODULE MEMOIRE (PAGINATION) ================\n");
        printf("1. Traduction d'adresse logique (page, deplacement, cadre)\n");
        printf("2. Simulation remplacement FIFO (etapes + defauts)\n");
        printf("3. Simulation remplacement LRU (etapes + defauts)\n");
        printf("4. Simulation remplacement Optimal (etapes + defauts)\n");
        printf("5. TLB : taux de succes et calcul de l'EAT\n");
        printf("6. Demo pas a pas : eviction FIFO puis LRU\n");
        printf("7. Bonus : test allocateur contigu First-Fit\n");
        printf("0. Retour au menu principal\n");
        choix = lire_int("Votre choix : ");

        switch (choix) {
            case 1: {
                int ps = lire_int("Taille de page (ex: 64) : ");
                int nf = lire_int("Nombre de cadres (ex: 4) : ");
                int te = lire_int("Taille TLB (ex: 4) : ");
                mem_pagination_init(&ctx, ps, nf, te, ctx.tlb_access_ns, ctx.mem_access_ns);
                int nb_addr = lire_int("Combien d'adresses logiques traduire ? ");
                for (int i = 0; i < nb_addr; i++) {
                    int addr = lire_int("Adresse logique : ");
                    int page = 0, off = 0, frame = 0, phys = 0;
                    bool hit = false, fault = false;
                    if (mem_translate_address(&ctx, addr, &page, &off, &frame, &phys, &hit, &fault)) {
                        printf("Addr %d -> page=%d, deplacement=%d, cadre=%d, addr_phys=%d | TLB=%s | %s\n",
                               addr, page, off, frame, phys,
                               hit ? "HIT" : "MISS",
                               fault ? "DEFAUT DE PAGE" : "OK");
                    }
                }
                break;
            }
            case 2:
                saisir_et_simuler_remplacement(1);
                break;
            case 3:
                saisir_et_simuler_remplacement(2);
                break;
            case 4:
                saisir_et_simuler_remplacement(3);
                break;
            case 5: {
                double t_tlb = lire_double("Temps acces TLB (ns, ex: 10) : ");
                double t_mem = lire_double("Temps acces memoire (ns, ex: 100) : ");
                ctx.tlb_access_ns = t_tlb;
                ctx.mem_access_ns = t_mem;
                int n_ref = lire_int("Nombre d'acces pages pour tester le TLB : ");
                int ps_bak = ctx.page_size;
                ctx.page_size = 1;
                for (int i = 0; i < n_ref; i++) {
                    int p = lire_int("Numero de page : ");
                    bool hit = false, fault = false;
                    mem_translate_address(&ctx, p, NULL, NULL, NULL, NULL, &hit, &fault);
                    printf("Acces page %d -> TLB %s\n", p, hit ? "HIT" : "MISS");
                }
                ctx.page_size = ps_bak;
                int total = ctx.tlb_hits + ctx.tlb_misses;
                double taux = (total > 0) ? (100.0 * (double)ctx.tlb_hits / (double)total) : 0.0;
                printf("Taux de succes TLB : %.2f%% (%d/%d) | EAT = %.2f ns\n",
                       taux, ctx.tlb_hits, total, mem_compute_eat(&ctx));
                break;
            }
            case 6:
                lancer_demo_fifo_lru();
                break;
            case 7: {
                MemBlock *m = mem_init(ESINOS_MEM_SIZE);
                mem_alloc(m, 1, 200);
                mem_alloc(m, 2, 300);
                mem_print(m);
                mem_free(m, 1);
                mem_print(m);
                mem_destroy(m);
                break;
            }
            case 0:
                printf("Retour au menu principal...\n");
                break;
            default:
                printf("Choix invalide.\n");
                break;
        }
    }
}

/* Bonus : allocateur contigu First-Fit */
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
