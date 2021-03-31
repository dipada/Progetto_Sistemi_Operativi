/*
È presente un processo “master” che raccoglie le statistiche delle varie richieste eseguite.
Ogni secondo viene stampato a terminale lo stato di occupazione delle varie celle.
Alla fine della simulazione vengono stampati:
• numero viaggi (eseguiti con successo, inevasi e abortiti)
• la mappa con evidenziate le SO_SOURCES sorgenti e le SO_TOP_CELLS celle più attraversate
• il processo taxi che
1. ha fatto più strada (numero di celle) di tutti
2. ha fatto il viaggio più lungo (come tempo) nel servire una richiesta
3. ha raccolto più richieste/clienti
*/

#include "master.h"



int main(int argc, char **argv){

int status, shmid;
pid_t pid;
map *city_map;

/* Creazione shm*/
if((shmid = shmget(SHMKEY, sizeof(map), IPC_CREAT | 0666)) == -1){
    ERROR_EXIT
}

/* crea e inizializza la mappa */
switch (pid = fork()){
    case -1:
        ERROR_EXIT

    case 0: /* ----- codice figlio ----- */
            /* crea e inizializza la mappa  */
        if(execl("mappa/map", "map", (char *) NULL)){
            ERROR_EXIT
        }

    default:
        wait(&status);
        if(check_status(status)){
            fprintf(stderr,"Error: can't generate map\n");
            exit(EXIT_FAILURE);
        }
}

/* attach della shm */
if( (city_map = (map *) shmat(shmid, NULL, 0)) == (void *) -1){
    ERROR_EXIT
}



/* TODO detach shm e cancellazione */





print_map(city_map);

exit(EXIT_SUCCESS);
}
