#include <stdio.h>
#include <string.h>
#include "fs.h"
#include "saisie.h"

#define FICHIER_DISQUE "data/disk.img" /*on donne le chemin du fichier de disque*/

static void lire_ligne(const char *invite, char *buf, int taille) {
    printf("%s", invite);
    if (fgets(buf, taille, stdin) == NULL) { /*lors d'un CTRL+D*/
        buf[0] = '\0';
        return;
    }
    buf[strcspn(buf, "\n")] = '\0'; /*on supprime le \n par un signe de fin de chaine*/
}

static int trouver_entree(const Disque *d, const char *nom) { /*fonction pour trouver l'entree dans le repertoire*/
    for (int i = 0; i < NB_INODES; i++) {
        if (d->rep[i].inode != -1 && strcmp(d->rep[i].nom, nom) == 0) { /*on compare le nom de l'entree avec le nom du fichier et on verifie qu'il n'est pas vide*/
            return i; /*si on trouve l'entree on retourne son indice*/
        }
    }
    return -1; /*si on ne trouve pas l'entree on retourne -1*/
}

static int inode_libre(const Disque *d) { /*fonction pour trouver un inode libre*/
    for (int i = 0; i < NB_INODES; i++) {
        if (d->inodes[i].utilise == 0) { /*on verifie si l'inode est libre*/
            return i; /*si on trouve un inode libre on retourne son indice*/
        }
    }
    return -1; /*si on ne trouve pas d'inode libre on retourne -1*/
}

static int bloc_libre(const Disque *d) { /*fonction pour trouver un bloc libre*/
    for (int i = 0; i < NB_BLOCS; i++) {
        if (d->bitmap[i] == 0) { /*on verifie si le bloc est libre*/
            return i; /*si on trouve un bloc libre on retourne son indice*/
        }
    }
    return -1; /*si on ne trouve pas de bloc libre on retourne -1*/
}

static int creer_fichier(Disque *d, const char *nom, const char *contenu){
    int taille = strlen(contenu);
    int nb_blocs = (taille + TAILLE_BLOC - 1) / TAILLE_BLOC; /*on calcule le nombre de blocs necessaires pour stocker le contenu*/

    if(strlen(nom) == 0 || strlen(nom) >= TAILLE_NOM) { /*on verifie que le nom du fichier n'est pas trop long et non vide*/
        printf("Erreur : le nom doit faire entre 1 et %d caracteres\n", TAILLE_NOM - 1);
        return -1;
    }
    if (trouver_entree(d, nom) != -1) { /*on verifie que le fichier n'existe pas deja*/
        printf("Erreur : le fichier %s existe deja\n", nom);
        return -1;
    }
    if (nb_blocs > Max_Blocs_Fichier) { /*on verifie que le fichier n'est pas trop gros*/
        printf("Erreur : le fichier %s est trop gros (max %d octets)\n", nom, Max_Blocs_Fichier * TAILLE_BLOC);
        return -1;
    }
    int n = inode_libre(d); /*on cherche un inode libre*/
    if (n == -1) { /*si on ne trouve pas d'inode libre*/
        printf("Erreur : plus d'inodes disponibles (%d fichiers max)\n", NB_INODES);
        return -1;
    }
    int libres = 0; /*on compte le nombre de blocs libres*/
    for (int i = 0; i < NB_BLOCS; i++) {
        if (d->bitmap[i] == 0) {
            libres++;
        }
    }
    if (libres < nb_blocs) { /*si le nombre de blocs libres est inferieur au nombre de blocs necessaires pour stocker le contenu*/
        printf("Erreur : plus de blocs disponibles (%d octets max)\n", NB_BLOCS * TAILLE_BLOC);
        return -1; 
    }
    for (int k = 0; k < nb_blocs; k++) { /*on cherche les blocs libres et on les associe a l'inode*/
        int b = bloc_libre(d);
        d->bitmap[b] = 1; /*on marque le bloc comme utilise*/
        d->inodes[n].blocs[k] = b; /*on associe le bloc a l'inode*/
    }
    for (int pos = 0; pos < taille; pos++) { /*on ecrit le contenu dans les blocs associes a l'inode*/
        int b = d->inodes[n].blocs[pos / TAILLE_BLOC]; /*on calcule le bloc correspondant a la position du caractere*/
        d->donnes[b][pos % TAILLE_BLOC] = contenu[pos]; /*on ecrit le caractere dans le bloc*/
    }
    d->inodes[n].utilise = 1; /*on marque l'inode comme utilise*/
    d->inodes[n].taille = taille; /*on met la taille du fichier dans l'inode*/
    strcpy(d->rep[n].nom, nom); /*on met le nom du fichier dans l'entree du repertoire*/
    d->rep[n].inode = n; /*on associe l'inode a l'entree du repertoire*/

    printf("%s créé : inode %d, %d octets, %d blocs\n", nom, n, taille, nb_blocs);
    for (int k = 0; k < nb_blocs; k++) {
        printf("  bloc %d\n", d->inodes[n].blocs[k]);
    }
    printf("\n");
    return 0;
}

static void formater(Disque *d) { /*fonction pour formater le disque*/
    d->sb.taille_bloc = TAILLE_BLOC; /*on met les caracteristiques du disque*/
    d->sb.nb_blocs = NB_BLOCS;
    d->sb.nb_inodes = NB_INODES;

    for (int i = 0; i < NB_BLOCS; i++) { /*on met le bitmap a 0 pour dire que les blocs sont libres*/
        d->bitmap[i] = 0;
        for (int j = 0; j < TAILLE_BLOC; j++) {
            d->donnes[i][j] = '\0'; /*on met les donnees a 0 pour dire qu'il n'y a pas de donnees*/
        }
    }

    for (int i = 0; i < NB_INODES; i++) { /*on met les caracteristiques des inodes a 0 pour dire qu'ils sont libres et qu'aucune donnees n'y est stockée*/
        d->inodes[i].utilise = 0;
        d->inodes[i].taille = 0;
        for (int j = 0; j < Max_Blocs_Fichier; j++) {
            d->inodes[i].blocs[j] = -1;/*on met les blocs a -1 pour dire qu'ils sont libres*/
        }
        d->rep[i].nom[0] = '\0'; /*on met le nom a \0 car c'est une chaine de caracteres pour dire qu'il n'y a pas de nom*/
        d->rep[i].inode = -1;/*l'inode est a -1 pour dire qu'il n'y a pas d'inode associe*/
    }
}

static void sauver(const Disque *d) { /*fonction pour sauver le disque dans un fichier*/
    FILE *f = fopen(FICHIER_DISQUE, "wb"); /*on ouvre le fichier en mode ecriture binaire*/
    if (f == NULL) {
        perror(FICHIER_DISQUE);/*on associe un message d'erreur qui donne l'arborescence du fichier*/
        return;
    }
    fwrite(d, sizeof(Disque), 1, f); /*on ecrit le disque dans le fichier*/
    fclose(f); /*on ferme le fichier*/
}

static void charger(Disque *d) { /*fonction pour charger le disque depuis un fichier*/
    FILE *f = fopen(FICHIER_DISQUE, "rb"); /*on ouvre le fichier en mode lecture binaire*/
    if (f == NULL) {
        printf("Le disque n'existe pas, formatage du disque...\n");
        formater(d); /*si le fichier n'existe pas on formate le disque*/
        sauver(d); /*on sauve le disque dans le fichier*/
        return;
    }
    if (fread(d, sizeof(Disque), 1, f) != 1) { /*on lit le disque depuis le fichier*/
        printf("Erreur de lecture du disque, formatage du disque...\n");
        formater(d); /*si la lecture echoue on formate le disque*/
        fclose(f);/*on ferme le fichier*/
        sauver(d); /*on sauve le disque dans le fichier*/
        return;
    }
    fclose(f); /*on ferme le fichier*/
}

static void lire_fichier(const Disque *d, const char *nom){
    int e = trouver_entree(d, nom);/*on cherche le nom dans le repertoire*/
    if (e == -1){/*Si on ne trouve pas le nom on affiche un message d'erreur*/
        printf("Erreur : le fichier %s n'existe pas\n", nom);
        return;
    }
    int n = d->rep[e].inode;/*a partir du repertoire on donne le numero d'inode*/
    printf("--- %s (inode %d, %d octets) ---\n", nom, n, d->inodes[n].taille);/*on affiche les données associés*/

    for (int pos = 0; pos < d->inodes[n].taille; pos++) {/*on parcourt l'inode*/
        int b = d->inodes[n].blocs[pos / TAILLE_BLOC]; /*on divise le blocs en morceaux*/
        putchar(d->donnes[b][pos % TAILLE_BLOC]);/*On sort le caractere a la position b*/
    }
    printf("\n");
}

static void lister(const Disque *d){/*liste le contenu d'un disque*/
    printf("%-15s | Inode | Taille\n", "Nom");/*affiche le format du tableau*/
    printf("----------------+-------+-------\n");
    int nb = 0;
    for (int i = 0; i < NB_INODES; i++) {/*On parcourt les inodes*/
        if (d->rep[i].inode != -1) {/*tant que l'inode existe*/
            int n = d->rep[i].inode;/*n correspond a l'inode qui existe*/
            printf("%-15s | %5d | %6d\n", d->rep[i].nom, n, d->inodes[n].taille);/*On affiche le nom du repertoire et la taille de l'inode */
            nb++;/*a chaque fichier trouver on incremente*/
        }
    }
    if (nb == 0){/*Si on ne trouve aucun fichier dans le repertoire on affiche qu'il est vide*/
        printf("(aucun fichier)\n");
    }
}

static int supprimer_fichier(Disque *d, const char *nom) {
    int e = trouver_entree(d, nom);/*on cherche le nom dans le repertoire*/
    if (e == -1){/*on check si le nom n'existe pas et si c'est le cas on l'affiche*/
        printf("Erreur : le fichier %s n'existe pas\n", nom);
        return -1;
    }
    int n = d->rep[e].inode;/*on stocke l'etat de l'inode du fichier choisi*/

    for (int k = 0;k < Max_Blocs_Fichier; k++){/*on parcourt les blocs de l'inode*/
        int b = d->inodes[n].blocs[k];/*on stock l'indice du bloc*/
        if (b != 1){
            d->bitmap[b] = 0;/*on montre que l'emplacement est vide sur la bitmap*/
            d->inodes[n].blocs[k] = -1;/*on montre que le bloc est vide*/
        }
    }
    d->inodes[n].utilise = 0;/*on montre que l'inode n'est pas utilisé*/
    d->inodes[n].taille = 0;/*on montre que son contenu est vide*/

    d->rep[e].nom[0] = '\0';/*on vide le nom associé dans le repertoire*/
    d->rep[e].inode = -1;/*de meme pour l'inode*/

    printf("%s supprime (inode %d libere)\n", nom, n);
    return 0;


}

static void afficher_inodes(const Disque *d){
    printf("Inode | Utilise | Taille | Blocs\n");/*print le format du tableau*/
    printf("------+---------+--------+------------\n");
    for (int i = 0; i < NB_INODES; i++){/*On parcourt les inodes*/
        if(d->inodes[i].utilise){/*Lorsqu'on on voit qu'elle est utilisé*/
            printf("%5d |   oui   | %6d |", i, d->inodes[i].taille);/*on affiche son indice l'indication qu'elle est utilisé et sa taille*/
            for (int k = 0; k < Max_Blocs_Fichier; k++){/*On parcourt les blocs*/
                if(d->inodes[i].blocs[k] != -1){/*Si le bloc est utilisé*/
                    printf(" %d", d->inodes[i].blocs[k]);/*on affiche les cases de bloc utilisé*/
                }
            }
            printf("\n");
        } else {
            printf("%5d |   non   |      - | -\n", i);/*Autrement on affiche l'indice de l'inode vide en montrant qu'il n'est pas utilisé*/
        }
    }
}

static void afficher_bitmap(const Disque *d) {
    int libres = 0;/*compte les blocs libre*/
    printf("Bitmap des blocs (1 = occupe, 0 = libre) :\n");/* legende de l'affichage*/
    for (int i = 0; i < NB_BLOCS; i++){/*on parcourt les blocs*/
        if (i % 16 == 0){/*on check si c'est le premier element et on affiche le debut*/
            printf("Blocs %2d-%2d : ", i, i + 15);
        }
        printf("%d ", d->bitmap[i]);/*on ajoute l'etat de chaque element a la bitmap*/
        if (d->bitmap[i] == 0) {
            libres++;/*incremente a chaque fois qu'on voit un bloc vide*/
        }
        if (i % 16 == 15){/*au bout de 15 blocs on passe a la ligne */
            printf("\n");
        }
    }
    printf("Blocs libres : %d / %d\n", libres,NB_BLOCS);/*on montre combien de blocs sont libres sur le total*/
}
void menu_fs(void)
{
    Disque d;/*on crée le disque*/
    charger(&d);/*on charge le disque depuis le fichier*/

    int choix = -1;
    while (choix != 0) { /*tant que l'utilisateur ne choisit pas de quitter*/
        printf("--- Système de fichiers ---\n");
        printf("1. Créer un fichier\n");
        printf("2. Lire un fichier\n");
        printf("3. Lister les fichiers (ls)\n");
        printf("4. Supprimer un fichier\n");
        printf("5. Table des inodes\n");
        printf("6. Bitmap des blocs\n");
        printf("7. Formater le disque\n");
        printf("0. Retour au menu principal\n");
        if (!lire_entier("Choix : ", 0, 7, &choix)){
            return;
        }

        char nom[64]; /* un peu plus grand que Taille_nom pour pouvoir detecter un nom trop long*/
        char contenu[300]; /* un peu plus grand que le maximum de Max_Blocs_Fichier * Taille_bloc pour pouvoir detecter un contenu trop long*/
        
        switch (choix) {
            case 1:
                lire_ligne("Nom du fichier : ", nom, sizeof(nom));
                lire_ligne("Contenu : ", contenu, sizeof(contenu));
                if (creer_fichier(&d, nom, contenu) == 0) { /*si la creation du fichier reussit*/
                    sauver(&d); /*on sauve le disque dans le fichier*/
                }
                break;
            case 2:
                lire_ligne("Nom du fichier : ", nom, sizeof(nom));/*lis le nom du fichier*/
                lire_fichier(&d, nom);/*affiche le fichier a partir du nom*/
                break;
            case 3:
                lister(&d);
                break;
            case 4:
                lire_ligne("Nom du fichier : ", nom, sizeof(nom));/*lis le nom du fichier*/
                if (supprimer_fichier(&d, nom) == 0) {/*Lorsque la suppression réussit*/
                    sauver(&d);/*on sauvegarde le disque*/
                }
                break;
            case 5:
                afficher_inodes(&d);
                break;
            case 6:
                afficher_bitmap(&d);
                break;
            case 7:
                int ok;
                if (lire_entier("Effacer Tous les fichiers ? (1 = oui, 0 = non) : ", 0, 1, &ok) && ok == 1){/*Demande l'input de l'utilisateur et check si il nous donne 1 qui veut dire oui pour le formatage*/
                    formater(&d);/*on formate*/
                    sauver(&d);/*on sauvegarde*/
                    printf("Disquer formate.\n");/*on affiche que le disque a été formaté*/
                }
                break;
        }
}
}