#include <stdio.h>

int lire_entier(const char *invite, int min, int max, int *resultat) {/*fonction pour lire un entier avec une invite, une valeur minimale et maximale, et stocker le résultat dans un pointeur*/

    char ligne[64];
    
    while (1) { /*se repete jusqu'a ce que la valeur soit valide*/
        printf("%s", invite);/*print le string voulue*/

        if (fgets(ligne, sizeof(ligne), stdin) == NULL) {/*termine la saisie lorsque l'utilisateur appuie sur Ctrl+D*/
            printf("\n");
            return 0;
        }

        int valeur; /*variable pour stocker la valeur saisie*/
        if (sscanf(ligne, "%d", &valeur) != 1 || valeur < min || valeur > max) {/*check si la valeur est valide et dans l'intervalle souhaité*/
            printf("Erreur : veuillez entrer un entier entre %d et %d.\n", min, max);/*message d'erreur rappelant l'intervalle souhaité*/
            continue;
        }
        
        *resultat = valeur;/*met la valeur dans le pointeur*/
        return 1;
    }
    }
    