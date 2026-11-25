#ifndef ESINOS_REPLACEMENT_H
#define ESINOS_REPLACEMENT_H

/* Défini une seule fois : si un autre header (ex. memory.h) le définit déjà, on garde le sien */
#ifndef MAX_FRAMES
#define MAX_FRAMES 16
#endif

/* Chaque fonction simule l'algorithme sur la chaîne de références refs (n éléments),
   avec num_frames cadres. Retourne le nombre de défauts de page, ou -1 si les
   arguments sont invalides. Si verbose vaut 1, affiche l'état des cadres pas à pas. */
int fifo(const int *refs, int n, int num_frames, int verbose);
int lru(const int *refs, int n, int num_frames, int verbose);
int optimal(const int *refs, int n, int num_frames, int verbose);

#endif