#include <stdio.h>
#include "process.h"



int main(){

	Process p = {1, 0, 5, 3, 5, -1, -1};
	printf("esinos : P%d arrive a t=%d, duree=%d\n", p.pid, p.arrival, p.duration);
	return 0;
}
