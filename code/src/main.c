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

	for(int i = 0; i < n; i++){
		printf("P%d : debut = %d, fin = %d\n", procs[i].pid, procs[i].start, procs[i].finish);
	}


	gantt_liberer(&g);
	return 0;
}
