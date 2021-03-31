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
#include "../mappa/mappa.h"

void sigint_h(int s){
    printf("Handler sono %d\n", getpid());
    fflush(stdout);
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv){

   signal(SIGINT, sigint_h);
    printf("PAD sono %d\n", getpid());
    if(fork() == 0){
        if(execl("mappa/map", "map", (char *) NULL)  == -1){
            ERROR_EXIT
        }
    
    }
    printf("finito3\n");
    
exit(EXIT_SUCCESS);
}
