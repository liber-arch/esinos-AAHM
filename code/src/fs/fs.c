#include <stdio.h>
#include "fs.h"

#define FICHIER_DISQUE "data/disk.img" /*on donne le chemin du fichier de disque*/

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

void menu_fs(void)
{
    Disque d;/*on crée le disque*/
    charger(&d);/*on charge le disque depuis le fichier*/
    printf("Disque : %d blocs de %d octets, %d inodes\n", d.sb.nb_blocs, d.sb.taille_bloc, d.sb.nb_inodes);/*On affiche les données associées au disque*/
}