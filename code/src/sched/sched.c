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

void simuler(Process procs[], int n, Algo algo, int quantum, Gantt *g){
    (void)quantum;

    for (int i = 0; i < n; i++) {
        procs[i].remaining = procs[i].duration;
        procs[i].start = -1;
        procs[i].finish = -1;
    }

    int t = 0;
    int termines = 0;
    int courant = -1;

    while (termines < n){

        if (courant == -1) {
            courant = choisir(procs, n, t, algo);
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


        if (procs[courant].remaining == 0) {
            procs[courant].finish = t + 1;
            termines ++;
            courant = -1;
        }

        t++;

    }
    
}