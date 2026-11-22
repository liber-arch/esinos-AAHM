#include <stdio.h>
#include "sched.h"



int main(){

	Process procs[MAX_PROCS];
	int n = charger_processus("data/procs.txt", procs, MAX_PROCS);
	if (n == -1){
		return 1;
	}

	printf("%d processus chargés :\n", n);
	afficher_processus(procs, n);

	Gantt g;
	gantt_init(&g);
	for (int t = 0; t < 5;t++) gantt_ajouter(&g,1,t);
	for (int t = 5; t < 8;t++) gantt_ajouter(&g, 2, t);
	gantt_ajouter(&g, -1, 8);
	for (int t = 9; t < 11; t++) gantt_ajouter(&g, 3, t);
	printf("\nTest du Gantt :\n");
	gantt_afficher(&g);
	gantt_liberer(&g);

	return 0;
}
