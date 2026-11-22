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
	return 0;
}
