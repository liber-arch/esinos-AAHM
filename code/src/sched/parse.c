#include <stdio.h>
#include "sched.h" /*importe la definition des types */

int charger_processus(const char *chemin, Process procs[], int max) { /*fonction de chargement des processus */
    FILE *f = fopen(chemin, "r");/*renvoie un pointeur sur le fichier ou NULL en cas d'erreur  et "r" signifie lecture seule*/
    if (f == NULL) { /*si le fichier renvoie NULL */
        perror("Erreur lors de l'ouverture du fichier");/*affiche le message d'erreur systeme*/
        return -1; /*montre l'erreur */
    }

    char ligne[256];/*tableau de caracteres pour stocker chaque ligne du fichier*/
    int n = 0;/*compteur du nombre de processus lus*/
    int num_ligne = 0;/*numéro de la ligne en cours de lecture*/

    while (fgets(ligne,sizeof(ligne), f) != NULL ) {/*lis des lignes du fichier f une par une tant qu'elles not sont pas vides*/
        num_ligne++;/*incremente a chaque ligne*/
        if (ligne[0] == '#' || ligne[0] == '\n') { /*ignore les lignes de commentaires ou vides*/
            continue; 
        }

        Process p;/*initialise un processus p*/
        int lus = sscanf(ligne, "%d %d %d %d", &p.pid, &p.arrival, &p.duration, &p.niceness);/*lit les valeurs du processus à partir de la ligne avec sscanf*/
        if (lus != 4 || p.arrival < 0 || p.duration <= 0) { /*si le format est incorrect ou les valeurs sont invalides on a un message d'erreur*/
            fprintf(stderr, "Erreur de format à la ligne %d : %s on attend pid arrivee duree niceness avec arrivee non nulle et duree positive", num_ligne, ligne);/*affiche le message d'erreur sur stderr*/
            fclose(f);/*on ferme le fichier*/
            return -1;
        }
        if (n >= max) {/*verifie qu'on ne depasse pas la limite de processus*/
            fprintf(stderr, "Erreur : le nombre de processus dépasse la limite de %d\n", max);
            fclose(f);
            return -1;
        }

        p.remaining = p.duration;/*au depart le temps et restant est egale a la duree totale*/
        p.start = -1;/*on considere -1 comme signe d'un processus non demarre*/
        p.finish = -1;/*de meme pour un processus qui n'est pas fini*/

        procs[n] = p;/*ajoute le processus au tableau des processus*/

        n++;/*incremente l'indice du tableau*/


    
    }

    fclose(f);
    return n;/*retourne le nombre de processus charges*/
}

void afficher_processus(const Process procs[], int n) {/*affiche un processus a partir du tableau des processus et l'indice auquel il est situé*/
    printf("PID | Arrivee | Duree | Niceness \n");
    printf("----+---------+-------+----------\n");
    for (int i = 0; i < n; i++) {
        printf("P%-2d | %7d | %5d | %8d\n", procs[i].pid, procs[i].arrival, procs[i].duration, procs[i].niceness);
    }/*formatage de l'affichage avec les valeurs associées au processus*/
}