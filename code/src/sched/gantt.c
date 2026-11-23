#include <stdio.h>
#include <stdlib.h> /*importe les fonctions de memoire*/
#include "sched.h"

void gantt_init(Gantt *g) {/*fonction qui initialise la structure Gantt */
    g->seg = NULL; /*le segment initial est vide* et on assigne cela aux champs du Gantt pointé avec ->*/
    g->n = 0; /*on initialise le nombre de segments à 0*/
    g->cap = 0;/*on initialise la capacite du tableau à 0*/
}

void gantt_ajouter(Gantt *g, int pid, int t){/*fonction qui ajoute un segment au diagramme de Gantt avec le PID du processus et le temps t */
    if (g->n > 0 && g->seg[g->n - 1].pid == pid && g->seg[g->n - 1].fin == t) {/*check si le segment actuel est du même processus et s'il se termine au même temps */
     g ->seg[g->n - 1].fin = t + 1;/*allonge le segment */
     return;
    }
    if (g->n == g->cap) {/*check si le tableau est plein */
        int nouvelle_cap = (g->cap == 0) ? 8 : g->cap * 2;/*double la capacité du tableau ou initialise à 8 si c'est le premier segment */
        Segment *tmp = realloc(g->seg, nouvelle_cap * sizeof(Segment));/*agrandit la capacité du tableau en conservant son contenu  */
        if (tmp == NULL) { /*si la reallocation échoue */
            perror("realloc");/*message d'erreur*/
            exit(EXIT_FAILURE);/*arrete le programme*/
        
        }
    g->seg = tmp;/*on adopte le nouveau segment allongé*/
    g->cap = nouvelle_cap;/*de meme pour la nouvelle capacité*/
    }
    g->seg[g->n].pid = pid; /*on entre les valeurs de la case d'indice n pour le segment du Gantt*/
    g->seg[g->n].debut = t;
    g->seg[g->n].fin = t + 1;
    g->n++;

}
void gantt_afficher(const Gantt *g){/*fonction qui affiche le diagramme de Gantt */
    if(g->n == 0){/*check si le Gantt est vide*/
        printf("Gantt vide\n");
        return;
    }
    for(int i = 0; i < g->n; i++) {/*parcourt tous les segments du Gantt pour affiche une ligne*/
        if(g->seg[i].pid == -1) {/*check si le segment est un segment d'inactivité et affiche IDLE si oui*/
            printf("| IDLE ");
        }else {
            printf("| P%-4d", g->seg[i].pid);/*affiche le PID du segment*/
        }
    }
    printf("|\n");

    for (int i = 0; i < g->n; i++) {/*reparcourt tous les segments du Gantt pour afficher les temps de debut*/
        printf("%-7d", g->seg[i].debut);
    }
    printf("%d\n", g->seg[g->n - 1].fin);/*affiche le temps de fin du dernier segment*/
}

void gantt_liberer(Gantt *g){/*fonction qui libère la mémoire allouée pour le diagramme de Gantt */
    free(g->seg);/*rend la memoire allouee*/
    g->seg = NULL;/*remet le pointeur à NULL*/
    g->n = 0; /*remet le nombre de segments à 0*/
    g->cap = 0;/*remet la capacité maximale à 0*/
}