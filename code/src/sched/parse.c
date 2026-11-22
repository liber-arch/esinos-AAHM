#include <stdio.h>
#include "sched.h"

int charger_processus(const char *chemin, Process procs[], int max) {
    FILE *f = fopen(chemin, "r");
    if (f == NULL) {
        perror("Erreur lors de l'ouverture du fichier");
        return -1; //() Return -1 to indicate an error
    }

    char ligne[256];
    int n = 0;
    int num_ligne = 0;

    while (fgets(ligne,sizeof(ligne), f) != NULL && n < max) {
        num_ligne++;
        if (ligne[0] == '#' || ligne[0] == '\n') {
            continue; 
        }

        Process p;
        int lus = sscanf(ligne, "%d %d %d %d", &p.pid, &p.arrival, &p.duration, &p.niceness);
        if (lus != 4 || p.arrival < 0 || p.duration <= 0) {
            fprintf(stderr, "Erreur de format à la ligne %d : %s on attend pid arrivee duree niceness avec arrivee non nulle et duree positive", num_ligne, ligne);
            fclose(f);
            return -1;
        }
        if (n >= max) {
            fprintf(stderr, "Erreur : le nombre de processus dépasse la limite de %d\n", max);
            fclose(f);
            return -1;
        }

        p.remaining = p.duration;
        p.start = -1;
        p.finish = -1;

        procs[n] = p;

        n++;


    
    }

    fclose(f);
    return n;
}

void afficher_processus(const Process procs[], int n) {
    printf("PID | Arrivee | Duree | Niceness \n");
    printf("----+---------+-------+----------\n");
    for (int i = 0; i < n; i++) {
        printf("P%-2d | %7d | %5d | %8d\n", procs[i].pid, procs[i].arrival, procs[i].duration, procs[i].niceness);
    }
}