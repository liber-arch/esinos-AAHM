#include <stdio.h>
#include "sched.h"

void calculer_moyennes(const Process procs[], int n, double *attente_moy, double *rotation_moy){
   if (n == 0) {
    *attente_moy = 0;
    *rotation_moy = 0;
    return;
   }

   int somme_attente = 0;
   int somme_rotation = 0;

   for (int i = 0; i < n; i++) {
    int rotation = procs[i].finish - procs[i].arrival;
    int attente = rotation - procs[i].duration;
    somme_attente += attente;
    somme_rotation += rotation;
   }

   *attente_moy = (double)somme_attente / n;
   *rotation_moy = (double)somme_rotation / n;

}

void afficher_metriques(const Process procs[], int n){
    printf("PID | Arrivee | Duree | Debut | Fin | Rotation | Attente\n");
    printf("----+---------+-------+-------+-----+----------+---------\n");

    for (int i = 0; i < n; i++) {
        int rotation = procs[i].finish - procs[i].arrival;
        int attente = rotation - procs[i].duration;
        printf("P%-2d | %7d | %5d | %5d | %3d | %8d | %7d\n",procs[i].pid, procs[i].arrival, procs[i].duration, procs[i].start, procs[i].finish, rotation, attente);
    }

double attente_moy, rotation_moy;
calculer_moyennes(procs, n, &attente_moy, &rotation_moy);
printf("Moyenne d'attente: %.2f\n", attente_moy);
printf("Moyenne de rotation: %.2f\n", rotation_moy);
}