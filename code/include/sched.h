#ifndef ESINOS_SCHED_H /* empeche de lire deux fois ce fichier dans main*/
#define ESINOS_SCHED_H

#include "process.h" /*importe le type process*/

#define MAX_PROCS 64 /* definit le maximum de processus */

typedef struct {
    int pid;
    int debut;
    int fin;
} Segment;

typedef struct {
    Segment *seg;
    int n;
    int cap;
} Gantt;

typedef enum {
    ALGO_FCFS,
    ALGO_SJF,
    ALGO_PRIO,
    ALGO_RR
} Algo;


int charger_processus(const char *chemin, Process procs[], int max); /* charge un processus dans la table des processus */
void afficher_processus(const Process procs[], int n); /* affiche les processus dans la table des processus */

void gantt_init(Gantt *g);
void gantt_ajouter(Gantt *g, int pid, int t);
void gantt_afficher(const Gantt *g);
void gantt_liberer(Gantt *g);

void simuler(Process procs[], int n, Algo algo, int quantum, Gantt *g); /* simule l'ordonnancement des processus */


#endif
