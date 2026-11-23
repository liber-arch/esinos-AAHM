#include <stdio.h>
#include "sched.h"

void calculer_moyennes(const Process procs[], int n, double *attente_moy, double *rotation_moy){/*calcule l'attente moyenne et la rotation moyenne*/
   if (n == 0) {/*si le nombre de processus est 0 on initialise les moyennes à 0*/
    *attente_moy = 0;
    *rotation_moy = 0;
    return;
   }

   int somme_attente = 0;/*on initialise la somme des temps d'attente*/
   int somme_rotation = 0;/*on initialise la somme des temps de rotation*/

   for (int i = 0; i < n; i++) {/*boucle qui calcule la somme des temps d'attente et de rotation*/
    int rotation = procs[i].finish - procs[i].arrival;/*formule de rotation*/
    int attente = rotation - procs[i].duration;/*formule d'attente*/
    somme_attente += attente;
    somme_rotation += rotation;
   }

   *attente_moy = (double)somme_attente / n;/*on calcule la moyenne d'attente*/
   *rotation_moy = (double)somme_rotation / n;/*on calcule la moyenne de rotation*/

}

void afficher_metriques(const Process procs[], int n){/*affiche un tableau qui affiche les donnees des processus*/
    printf("PID | Arrivee | Duree | Debut | Fin | Rotation | Attente\n");
    printf("----+---------+-------+-------+-----+----------+---------\n");/*format d'affichage*/

    for (int i = 0; i < n; i++) {
        int rotation = procs[i].finish - procs[i].arrival;
        int attente = rotation - procs[i].duration;
        printf("P%-2d | %7d | %5d | %5d | %3d | %8d | %7d\n",procs[i].pid, procs[i].arrival, procs[i].duration, procs[i].start, procs[i].finish, rotation, attente);
    }/*boucle qui pour chaque processus calcule et affiche ses metriques*/

double attente_moy, rotation_moy;/*initialise les variables pour stocker les moyennes*/
calculer_moyennes(procs, n, &attente_moy, &rotation_moy);/*calcule les moyennes*/
printf("Moyenne d'attente: %.2f\n", attente_moy);/*affichage des moyennes*/
printf("Moyenne de rotation: %.2f\n", rotation_moy);
}