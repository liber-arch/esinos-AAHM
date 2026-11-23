#include <stdio.h>
#include "sched.h"

static int est_prioritaire(const Process *a, const Process *b, Algo algo){
    switch (algo) {
        case ALGO_SJF:
        if (a->duration != b->duration) {
            return a->duration < b->duration;
        }
        break;

        case ALGO_PRIO:
        if (a->niceness != b->niceness) {
            return a->niceness < b->niceness;
        }
        break;

        case ALGO_FCFS:
        default:
            break;
    }

    if (a->arrival != b->arrival) {
        return a->arrival < b->arrival;
    }
    return a->pid < b->pid;
}

static int choisir(const Process procs[], int n, int t, Algo algo){
    int elu = -1;
    for (int i = 0; i < n;i++) {
        if(procs[i].arrival > t || procs[i].remaining == 0) {
            continue;
        }
        if (elu == -1 || est_prioritaire(&procs[i], &procs[elu], algo)){
            elu = i;
        }
    }
    return elu;
}

typedef struct {
    int tab[MAX_PROCS];
    int tete;
    int taille;
} File;

static void file_init(File *f){
    f->tete = 0;
    f->taille = 0;
}

static void file_ajouter(File *f, int x){
    int fin = (f->tete + f->taille) % MAX_PROCS;
    f->tab[fin] = x;
    f->taille++;
}

static int file_retirer(File *f){
    int x = f->tab[f->tete];
    f->tete = (f->tete + 1) % MAX_PROCS;
    f->taille--;
    return x;
}

void simuler(Process procs[], int n, Algo algo, int quantum, Gantt *g){
    
    if (algo == ALGO_RR && quantum < 1){
        fprintf(stderr, "Quantum invalide (%d) : on utilise 1 par défaut\n", quantum);
        quantum = 1;
    } 

    for (int i = 0; i < n; i++) {
        procs[i].remaining = procs[i].duration;
        procs[i].start = -1;
        procs[i].finish = -1;
    }

    File file;
    file_init(&file);

    int t = 0;
    int termines = 0;
    int courant = -1;
    int tranche = 0;

    while (termines < n){

        if(algo == ALGO_RR) {
            for (int i = 0; i < n; i++) {
                if (procs[i].arrival == t) {
                    file_ajouter(&file, i);
                }
            }
        }

        if (courant != -1 && tranche == quantum) {
            file_ajouter(&file, courant);
            courant = -1;
        }

        if (courant == -1) {
            if (algo == ALGO_RR) {
                if (file.taille > 0) {
                    courant = file_retirer(&file);
                }
            } else {
                courant = choisir(procs, n, t, algo);
            }
            tranche = 0;
        }

        if (courant == -1) {
            gantt_ajouter(g, -1, t);
            t++;
            continue;
        }

        if (procs[courant].start == -1) {
            procs[courant].start = t;
        }
        gantt_ajouter(g, procs[courant].pid, t);
        procs[courant].remaining--;
        tranche++;


        if (procs[courant].remaining == 0) {
            procs[courant].finish = t + 1;
            termines ++;
            courant = -1;
        }

        t++;

    }
    
}