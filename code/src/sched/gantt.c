#include <stdio.h>
#include <stdlib.h>
#include "sched.h"

void gantt_init(Gantt *g) {
    g->seg = NULL;
    g->n = 0;
    g->cap = 0;
}

void gantt_ajouter(Gantt *g, int pid, int t){
    if (g->n > 0 && g->seg[g->n - 1].pid == pid && g->seg[g->n - 1].fin == t) {
     g ->seg[g->n - 1].fin = t + 1;
     return;
    }
    if (g->n == g->cap) {
        int nouvelle_cap = (g->cap == 0) ? 8 : g->cap * 2;
        Segment *tmp = realloc(g->seg, nouvelle_cap * sizeof(Segment));
        if (tmp == NULL) {
            perror("realloc");
            exit(EXIT_FAILURE);
        
        }
    g->seg = tmp;
    g->cap = nouvelle_cap;    
    }
    g->seg[g->n].pid = pid;
    g->seg[g->n].debut = t;
    g->seg[g->n].fin = t + 1;
    g->n++;

}
void gantt_afficher(const Gantt *g){
    if(g->n == 0){
        printf("Gantt vide\n");
        return;
    }
    for(int i = 0; i < g->n; i++) {
        if(g->seg[i].pid == -1) {
            printf("| IDLE ");
        }else {
            printf("| P%-4d", g->seg[i].pid);
        }
    }
    printf("|\n");

    for (int i = 0; i < g->n; i++) {
        printf("%-7d", g->seg[i].debut);
    }
    printf("%d\n", g->seg[g->n - 1].fin);
}

void gantt_liberer(Gantt *g){
    free(g->seg);
    g->seg = NULL;
    g->n = 0;
    g->cap = 0;
}