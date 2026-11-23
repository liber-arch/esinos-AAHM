#include <stdio.h>
#include "sched.h"

static int est_prioritaire(const Process *a, const Process *b, Algo algo){/*nous dit si un processus a est prioritaire par rapport à un processus b selon l'algorithme*/
    switch (algo) {
        case ALGO_SJF:
        if (a->duration != b->duration) {/*dans le cas du SJF on compare les durées */
            return a->duration < b->duration;
        }
        break;

        case ALGO_PRIO:
        if (a->niceness != b->niceness) {/*dans le cas du PRIO on compare les valeurs de niceness */
            return a->niceness < b->niceness;
        }
        break;

        case ALGO_FCFS:/*dans le cas du FCFS on utilise les comparaisons par défaut*/
        default:
            break;
    }

    if (a->arrival != b->arrival) {/*dans les cas oue le SJF et le prio ne suffisent pas et pour le FCFS on compare les arrivées*/
        return a->arrival < b->arrival;
    }
    return a->pid < b->pid;/*si ca ne suffit toujours pas, on compare les PIDs*/
}

static int choisir(const Process procs[], int n, int t, Algo algo){/*parmi les processus retourne le meilleur en fonction de l'algorithme*/
    int elu = -1;
    for (int i = 0; i < n;i++) {/*parcourt tous les processus pour trouver le meilleur selon l'algorithme*/
        if(procs[i].arrival > t || procs[i].remaining == 0) {/*si le processus n'est pas encore arrivé ou fini on passe au suivant*/
            continue;
        }
        if (elu == -1 || est_prioritaire(&procs[i], &procs[elu], algo)){/*si le processus est prioritaire par rapport à l'elu on le remplace*/
            elu = i;/*on met à jour l'indice du processus choisi a l'etape de la boucle ou on l'as trouvé*/
        }
    }
    return elu;
}

typedef struct {
    int tab[MAX_PROCS];
    int tete;/*indice de la tete de la file*/
    int taille;/*nombre d'elements dans la file*/
} File;/*la File contient un ensemble d'indices du tableau */

static void file_init(File *f){/*initialise la file*/
    f->tete = 0;
    f->taille = 0;
}

static void file_ajouter(File *f, int x){/*ajoute un element x à la file*/
    int fin = (f->tete + f->taille) % MAX_PROCS;/*donne l'indice fin qui donne la 1ere case libre dans la file en faisant le tour (on peut revenir en arriere grace au modulo)*/
    f->tab[fin] = x;/*met l'element x dans la case libre*/
    f->taille++;/*augmente la taille de la file*/
}

static int file_retirer(File *f){/*selon le meme principe que pour file_ajouter renvoie la tete de file en fin de file*/
    int x = f->tab[f->tete];
    f->tete = (f->tete + 1) % MAX_PROCS;
    f->taille--;
    return x;
}

void simuler(Process procs[], int n, Algo algo, int quantum, Gantt *g){/*fonction de simulation de l'algorithme sur les processus*/
    
    if (algo == ALGO_RR && quantum < 1){/*lorsqu'on on a un quantum < 1 on ne peut pas effectuer l'algorithme RR on affiche un message d'erreur et on met le quantum a 1*/
        fprintf(stderr, "Quantum invalide (%d) : on utilise 1 par défaut\n", quantum);
        quantum = 1;
    } 

    for (int i = 0; i < n; i++) {/*boucle qui initialise l'etat des processus */
        procs[i].remaining = procs[i].duration;/*duree restante = duree totale*/
        procs[i].start = -1;/*un processus non demarré a un start = -1*/
        procs[i].finish = -1;/*un processus non terminé a un finish = -1*/
    }

    File file;/*creation d'une file*/
    file_init(&file);/*initialise la file*/

    int t = 0;/*instant courant*/
    int termines = 0;/*nombre de processus terminés*/
    int courant = -1;/*indice du processus courant*/
    int tranche = 0;/*longueur de temps que le processus a passé en cours*/

    while (termines < n){/*tant qu'il reste des processus a traiter*/

        if(algo == ALGO_RR) {/*dans un algorithme round robin on ajoute les processus arrivés en fin de file*/
            for (int i = 0; i < n; i++) {
                if (procs[i].arrival == t) {
                    file_ajouter(&file, i);
                }
            }

        if (courant != -1 && tranche == quantum) {/*si le processus courant a utilisé toute sa tranche de temps on le met en fin de file*/
            file_ajouter(&file, courant);
            courant = -1;
        }
    }

        if (courant == -1) {/*lorsque le CPU est libre*/
            if (algo == ALGO_RR) {
                if (file.taille > 0) {
                    courant = file_retirer(&file);/*on retire la tete de file*/
                }
            } else {
                courant = choisir(procs, n, t, algo);/*sinon on choisit le meilleur processus en fonction de l'algo*/
            }
            tranche = 0;/*initialise la longueur de temps que le processus a passé en cours*/
        }

        if (courant == -1) {/*lorsque le CPU est libre*/
            gantt_ajouter(g, -1, t);/*on ajoute un segment IDLE dans le diagramme de Gantt*/
            t++;
            continue;
        }

        if (procs[courant].start == -1) {/*si le processus courant n'est pas demarré on le commence a l'instant t*/
            procs[courant].start = t;
        }
        gantt_ajouter(g, procs[courant].pid, t);/*on ajoute un segment dans le diagramme de Gantt avec le processus actuel*/
        procs[courant].remaining--;/*on retire une unité de temps de la durée restante*/
        tranche++;/*on incrémente la longueur de temps que le processus a passé en cours*/


        if (procs[courant].remaining == 0) {/*si le processus courant est terminé*/
            procs[courant].finish = t + 1;/*le processus est terminé à l'instant t + 1*/
            termines ++;/*on incremente le nombre de processus terminés*/
            courant = -1;/*le CPU redevient libre*/
        }

        t++;/*on incremente l'instant courant*/

    }
    
}