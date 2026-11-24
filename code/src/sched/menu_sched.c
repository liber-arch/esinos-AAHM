#include <stdio.h>
#include "sched.h"
#include "saisie.h"

#define NB_ALGOS 4 /*nombre d'algorithme a tester*/

static void lancer(Process procs[], int n, Algo algo, int quantum, const char *nom, double *attente_moy, double *rotation_moy){/*lance un algorithme sur un processus et affiche son gantt et ses metriques*/
	Gantt g;
	gantt_init(&g);

	simuler(procs, n, algo, quantum, &g);

	printf("\n=== %s ===\n", nom);
	gantt_afficher(&g);
	printf("\n");
	afficher_metriques(procs, n);

	calculer_moyennes(procs, n, attente_moy, rotation_moy);

	gantt_liberer(&g);
}



void menu_ordonnanceur(){ /*affichage et création du diagramme de GANTT*/

	Process procs[MAX_PROCS];/*on cree un tableau de processus*/
	int n = charger_processus("data/procs.txt", procs, MAX_PROCS);/*on invoque la fonction de chargement des processus a partir du fichier choisi*/
	if (n == -1){/*si on a pas de processus on s'arrete*/
		return;
	}

	printf("%d processus chargés :\n", n);/*affiche le nombre de processus chargé*/
	afficher_processus(procs, n);/*on affiche les processus a l'aide de la fonction*/

    int quantum;/*on declare le quantum pour l'algorithme RR*/
    if (!lire_entier("Entrez le quantum pour l'algorithme RR(1-100) : ", 1, 100, &quantum)) {/*si le quantum n'est pas dans l'intervalle choisie on s'arrete*/
        return;
    }

	char nom_rr[32];
	snprintf(nom_rr, sizeof(nom_rr), "RR (quantum=%d)", quantum);/*on affiche l'algorithme ainsi que le quantum choisi*/

	Algo algos[NB_ALGOS] = {ALGO_FCFS, ALGO_SJF, ALGO_PRIO, ALGO_RR};/*on crée le tableau d'algorithmes*/
	const char *noms[NB_ALGOS] = {"FCFS", "SJF", "PRIO", nom_rr};/*on crée le tableau de noms d'algorithmes*/
	double attentes[NB_ALGOS], rotations[NB_ALGOS];/*on crée les tableaux de temps d'attente et de rotation*/

	for (int i = 0; i < NB_ALGOS; i++){/*on lance chaque algorithme sur les processus et on stocke les résultats*/
		lancer(procs, n, algos[i], quantum, noms[i], &attentes[i], &rotations[i]);
	}

	int meilleur = 0;/*on initialise le meilleur algorithme a 0*/
	for (int i = 1; i < NB_ALGOS; i++){/*on compare les temps d'attente pour trouver le meilleur*/
		if (attentes[i] < attentes[meilleur]){
			meilleur = i;
		}
	}

	printf("\n=== Comparaison ===\n");/*on utilise ce code pour le format d'affichage*/
	printf("%-18s | %12s | %13s\n", "Algorithme", "Attente moy", "Rotation moy");
	printf("-------------------+--------------+--------------\n");
	for (int i = 0; i < NB_ALGOS; i++){
		printf("%-18s | %12.2f | %13.2f%s\n", noms[i], attentes[i], rotations[i], (i == meilleur) ? " <-- meilleur attente" : "");
	}
}
