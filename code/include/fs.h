#ifndef ESINOS_FS_H
#define ESINOS_FS_H

#define TAILLE_BLOC 64
#define NB_BLOCS 32
#define NB_INODES 8
#define Max_Blocs_Fichier 4
#define Taille_Nom 16

typedef struct { /*les caracteristiques du disque*/
    int taille_bloc;
    int nb_blocs;
    int nb_inodes;
} Superbloc;

typedef struct {/*on sait si l'inode est utilise combien de blocs il utilise et la taille du contenu en octets*/
    int utilise;
    int taille;
    int blocs[Max_Blocs_Fichier];
} Inode;

typedef struct {/*permet de stocker les informations sur les fichiers dans le repertoire*/
    char nom[Taille_Nom];
    int inode;
} EntreeRep;

typedef struct {/*permet de créer une simulation de disque*/
    Superbloc sb;
    int bitmap[NB_BLOCS];
    Inode inodes[NB_INODES];
    EntreeRep rep[NB_INODES];
    char donnes[NB_BLOCS][TAILLE_BLOC];
} Disque;

void menu_fs(void);

#endif