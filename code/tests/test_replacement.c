#include <stdio.h>
#include "replacement.h"

int main(void) {
    int refs[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1};
    int n = (int)(sizeof refs / sizeof refs[0]);

    printf("FIFO    : %d defauts (attendu 15)\n", fifo(refs, n, 3, 0));
    printf("LRU     : %d defauts (attendu 12)\n", lru(refs, n, 3, 0));
    printf("Optimal : %d defauts (attendu 9)\n", optimal(refs, n, 3, 0));

    printf("\nFIFO pas a pas (5 premieres references) :\n");
    fifo(refs, 5, 3, 1);
    return 0;
}