#include <stdio.h>
#include "sched.h"



int main(int argc, char *argv[]){

	const char *chemin = (argc > 1) ? argv[1] : "data/procs.txt";

	Process procs[MAX_PROCS];
	int n = charger_processus(chemin, procs, MAX_PROCS);
	if (n == -1){
		return 1;
	}

	printf("%d processus chargés :\n", n);
	afficher_processus(procs, n);

	Gantt g;
	gantt_init(&g);
	simuler(procs, n, ALGO_FCFS, 0, &g);

	printf("\n=== FCFS ===\n");
	gantt_afficher(&g);

	printf("\n");
	afficher_metriques(procs, n);


	gantt_liberer(&g);
	return 0;
}
