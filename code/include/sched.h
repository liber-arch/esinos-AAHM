#ifndef ESINOS_SCHED_H /* empeche de lire deux fois ce fichier dans main*/
#define ESINOS_SCHED_H

#include "process.h" /*importe le type process*/

#define MAX_PROCS 64 /* definit le maximum de processus */

typedef struct { /* definit la structure d'un segment composé du PID du processus concerné avec le temps de debut et de fin */
    int pid;
    int debut;
    int fin;
} Segment;

typedef struct {/* definit la structure d'un diagramme de Gantt  qui est composé de n segments et reserve cap case de memoire */
    Segment *seg;/*Comme on ne connait pas encore le nombre de segments, on utilise un pointeur */
    int n;
    int cap;
} Gantt;

typedef enum {/* definit les différents algorithmes d'ordonnancement */
    ALGO_FCFS,
    ALGO_SJF,
    ALGO_PRIO,
    ALGO_RR
} Algo;


int charger_processus(const char *chemin, Process procs[], int max); /* charge un processus dans la table des processus */
void afficher_processus(const Process procs[], int n); /* affiche les processus dans la table des processus */

void gantt_init(Gantt *g);/* initialise un diagramme de Gantt */
void gantt_ajouter(Gantt *g, int pid, int t)/* ajoute un segment au diagramme de Gantt */;
void gantt_afficher(const Gantt *g);/* affiche le diagramme de Gantt */
void gantt_liberer(Gantt *g);/* libere la memoire allouee pour le diagramme de Gantt */

void simuler(Process procs[], int n, Algo algo, int quantum, Gantt *g); /* simule l'ordonnancement des processus */

void calculer_moyennes(const Process procs[], int n, double *attente_moy, double *rotation_moy); /* calcule les moyennes d'attente et de rotation */
void afficher_metriques(const Process procs[], int n); /* affiche les metriques d'attente et de rotation */

void menu_ordonnanceur();/*affiche le menu de l'ordonnanceur*/

#endif
