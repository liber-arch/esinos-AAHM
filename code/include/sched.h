#ifndef ESINOS_SCHED_H /* empeche de lire deux fois ce fichier dans main*/
#define ESINOS_SCHED_H

#include "process.h" /*importe le type process*/

#define MAX_PROCS 64 /* definit le maximum de processus */


int charger_processus(const char *chemin, Process procs[], int max); /* charge un processus dans la table des processus */
void afficher_processus(const Process procs[], int n); /* affiche les processus dans la table des processus */

#endif
