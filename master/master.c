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

    int status, shm_map, shm_par, shm_stat, semid;
    pid_t pid;
    map *city_map;
    struct parameters *param;
    struct statistic *stat;

    union semun arg;
    struct sembuf sops[3];
    
    /* Creazione SHM */
    if((shm_map = shmget(SHMKEY_MAP, sizeof(map), IPC_CREAT | 0666)) == -1){
        ERROR_EXIT
    }
    if((shm_par = shmget(SHMKEY_PAR, sizeof(struct parameters), IPC_CREAT | 0666)) == -1){
        ERROR_EXIT
    }
    if((shm_stat = shmget(SHMKEY_STAT, sizeof(struct statistic), IPC_CREAT | 0666)) == -1){
        ERROR_EXIT
    }
    
    /* attach della shm */
    if((city_map = (map *) shmat(shm_map, NULL, 0)) == (void *) -1){
        ERROR_EXIT
    }
    if((param = (struct parameters *) shmat(shm_par, NULL, 0)) == (void *) -1){
        ERROR_EXIT
    }
    if((stat = (struct statistic *) shmat(shm_stat, NULL, 0)) == (void *) -1){
        ERROR_EXIT
    }
    
    /* allocazione dei semafori */
    if((semid = semget(SEMKEY, 3, IPC_CREAT | 0666)) == -1){
        ERROR_EXIT
    }
    /* inizializzazione semafori*/
    if((arg.array = (unsigned short *)malloc(sizeof(unsigned short)*3)) == NULL ){
        ERROR_EXIT
    }
    arg.array[SEM_MASTER] = 0;
    arg.array[SEM_SOURCE] = 1;
    arg.array[SEM_TAXI] = 1;
    if(semctl(semid, 0, SETALL, arg.array) == -1){
        ERROR_EXIT
    }
    /*
    if(semctl(semid, SEM_MASTER, SETVAL, arg) == -1){
        ERROR_EXIT
    }
    if(semctl(semid, SEM_SOURCE, SETVAL, arg) == -1){
        ERROR_EXIT
    }
    if(semctl(semid, SEM_TAXI, SETVAL, arg) == -1){
        ERROR_EXIT
    }*/
    printf("valore di semmaster %d, source %d, semtaxi %d\n", semctl(semid, SEM_MASTER, GETVAL), semctl(semid, SEM_SOURCE, GETVAL), semctl(semid, SEM_TAXI, GETVAL));


    switch (pid = fork()){
        case -1:
            ERROR_EXIT

        case 0: /* ----- codice figlio ----- */
            /* crea, inizializza la mappa e posiziona HOLES */
            if(execl("mappa/map", "map", (char *) NULL)){
                ERROR_EXIT
            }
        
        default: /* ----- codice del padre ----- */
            /* attende la terminazione e analizza lo status di uscita */
            wait(&status);
            if(check_status(status)){
                fprintf(stderr,"[%s]: Exiting due to error map...\n", __FILE__);
                exit(EXIT_FAILURE);
            }       
    }
    
    switch (pid = fork()){
        case -1:
            ERROR_EXIT

        case 0: /* ----- codice figlio ----- */
            /* posiziona SO_SOURCE e genera SO_TAXI  */
            if(execl("taxi/taxi", "taxi", (char *) NULL)){
                ERROR_EXIT
            }

        /* ----- codice del padre ----- */
            /* attende la terminazione e analizza lo status di uscita */
            wait(&status);
            if(check_status(status)){
                fprintf(stderr,"[%s]: Exiting due to error taxi...\n", __FILE__);
                exit(EXIT_FAILURE);
            }      
    }
    /*waitpid(0,NULL,0);*/


    /* la simulazione parte dopo che sia taxi sia source sono stati creati e inizializzati */
    
    /* aspetto che i semafori source e taxi diventano 0 e incremento quello master per far partire la simulazione */
    printf("master");
    sops[0].sem_num = SEM_SOURCE;
    sops[0].sem_op = 0;
    sops[0].sem_flg = 0;

    sops[1].sem_num = SEM_TAXI;
    sops[1].sem_op = 0;
    sops[1].sem_flg = 0;
    
    sops[2].sem_num = SEM_MASTER;
    sops[2].sem_op = 1;
    sops[2].sem_flg = 0;
    if(semop(semid, sops, 3) == -1){
        ERROR_EXIT
    }

    printf("master mi ha sbloccato!\n");


    /* ogni secondo stampa lo stato di occupazione delle celle */
    /*print_status_cells(city_map);*/
    
    printf("Richieste totali generate: %d\n", stat->n_request);

    /* stampa la mappa evidenziando HOLE, SOURCE e TOP_CELLS*/
    print_map(city_map);

    /*for(;;){
        print_status_cells(city_map);
        sleep(1);
    }*/
    
    wait(&status);
    /*print_status_cells(city_map);*/
    
    /* TODO rimozione semafori */

    /* detach e rimozione SHM */
    if(shmdt(city_map) == -1){
        ERROR_EXIT
    }
    if(shmdt(param) == -1){
        ERROR_EXIT
    }
    if(shmdt(stat) == -1){
        ERROR_EXIT
    }
    if(shmctl(shm_map,IPC_RMID, NULL) == -1){
        ERROR_EXIT
    }
    if(shmctl(shm_par,IPC_RMID, NULL) == -1){
        ERROR_EXIT
    }
    if(shmctl(shm_stat,IPC_RMID, NULL) == -1){
        ERROR_EXIT
    }

    exit(EXIT_SUCCESS);

}
