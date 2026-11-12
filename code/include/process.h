#ifndef ESINOS_PROCESS_H /* si le fichier n'as pas déja été lu/défini on passe a la suite autrement on passe
 directement a #endif */

#define ESINOS_PROCESS_H /* on assigne a ESINOS_PROCESS_H le fait qu'il est defini  */


typedef struct {  /* On definit le type des processus */
	int pid; /* variable associée au numéro du processus */
	int arrival; /*  instant ou le processus arrive dans la file */
	int duration; /*  durée nécessaire a l'execution du processus en entier */
	int niceness; /* priorité du processus (plus elle est petite plus la priorité est haute) */

	int remaining; /*  temps restant avant la fin du processus se met a jour pendant l'execution */
	int start; /* instant de debut d'execution (-1 si il n'as pas commencé) */
	int finish; /*  instant de fin (-1 si il n'est pas fini) */ 

} Process;

#endif 
