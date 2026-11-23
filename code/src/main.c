#include <stdio.h>
#include "sched.h"


static void lancer(Process procs[], int n, Algo algo, int quantum, const char *nom){
	Gantt g;
	gantt_init(&g);

	simuler(procs, n, algo, quantum, &g);

	printf("\n=== %s ===\n", nom);
	gantt_afficher(&g);
	printf("\n");
	afficher_metriques(procs, n);

	gantt_liberer(&g);
}



int main(int argc, char *argv[]){

	const char *chemin = (argc > 1) ? argv[1] : "data/procs.txt";

	Process procs[MAX_PROCS];
	int n = charger_processus(chemin, procs, MAX_PROCS);
	if (n == -1){
		return 1;
	}

	printf("%d processus chargés :\n", n);
	afficher_processus(procs, n);

	lancer(procs, n, ALGO_FCFS, 0, "FCFS");
	lancer(procs, n, ALGO_SJF, 0, "SJF");
	lancer(procs, n, ALGO_PRIO, 0, "PRIO");
	lancer(procs, n, ALGO_RR, 2, "RR (quantum=2)");

	return 0;
}
