#include <stdio.h>
#include <stdlib.h>
#include "sched.h"

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



int main(int argc, char *argv[]){

	const char *chemin = (argc > 1) ? argv[1] : "data/procs.txt";
	int quantum = (argc > 2) ? atoi(argv[2]) : 2;
	if (quantum < 1){
		fprintf(stderr, "Quantum invalide : %s on attend un nombre >= 1\n", argv[2]);
		return 1;
	}

	Process procs[MAX_PROCS];
	int n = charger_processus(chemin, procs, MAX_PROCS);
	if (n == -1){
		return 1;
	}

	printf("%d processus chargés :\n", n);
	afficher_processus(procs, n);

	char nom_rr[32];
	snprintf(nom_rr, sizeof(nom_rr), "RR (quantum=%d)", quantum);

	Algo algos[NB_ALGOS] = {ALGO_FCFS, ALGO_SJF, ALGO_PRIO, ALGO_RR};
	const char *noms[NB_ALGOS] = {"FCFS", "SJF", "PRIO", nom_rr};
	double attentes[NB_ALGOS], rotations[NB_ALGOS];

	for (int i = 0; i < NB_ALGOS; i++){
		lancer(procs, n, algos[i], quantum, noms[i], &attentes[i], &rotations[i]);
	}

	int meilleur = 0;
	for (int i = 1; i < NB_ALGOS; i++){
		if (attentes[i] < attentes[meilleur]){
			meilleur = i;
		}
	}

	printf("\n=== Comparaison ===\n");
	printf("%-18s | %12s | %13s\n", "Algorithme", "Attente moy", "Rotation moy");
	printf("-------------------+--------------+--------------\n");
	for (int i = 0; i < NB_ALGOS; i++){
		printf("%-18s | %12.2f | %13.2f%s\n", noms[i], attentes[i], rotations[i], (i == meilleur) ? " <-- meilleur attente" : "");
	}

	return 0;
}
