#include <stdio.h>
#include "sched.h"
#include "saisie.h"

int main () {
	int choix = 1;

	while (choix != 0) {
		printf("\n===== esinos =====\n");
		printf("1. Ordonnanceur\n");
		printf("2. Gestionnaire memoire\n");
        printf("3. Systeme de fichiers\n");
		printf("0. Quitter\n");
		if (!lire_entier("Choix : ", 0, 3, &choix)) {
			break;
		}

		switch (choix) {
			case 1:
				menu_ordonnanceur();
				break;
			case 2 :
				printf("Gestionnaire memoire non implémenté\n");
				break;
			case 3:
				printf("Système de fichiers non implémenté\n");
				break;
			case 0:
				printf("Vous allez quitter le menu\n");
				break;
		}

	}
	return 0;
}