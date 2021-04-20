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
#include "../taxi/taxi.h"

static int t = 1;



void master_handler(int sig){
    if(sig == SIGALRM){ /* scaduto il tempo della simulazione */
        
        kill(0, SIGTERM);    
        
        printf(CRED"%s ricevuto SIGALRM"CDEFAULT"\n", __FILE__);
        printf(CRED"TEMPO FINITO"CDEFAULT"\n");
    }
    if(sig == SIGTERM){
        /*printf("%s Ricevuto SIGTERM\n", __FILE__);*/
        t = 0;
    }

    if(sig == SIGCHLD){
        if(t){
            printf(CGREEN"ricevuto SIGUSR1."CDEFAULT" figlio terminato %d\n", wait(NULL));
            switch(fork()){
                case -1:
                    ERROR_EXIT
                case 0:
                    if(execl("taxi/taxi", "taxi", (char*) NULL) == -1){
                        ERROR_EXIT
                    }
                default:
                    printf("Creazione avvenuta\n");
            }
        }
    }
}



int main(int argc, char **argv){

    int status, shm_map, shm_par, shm_stat, semid, qid, i;
    pid_t pid;
    map *city_map;
    struct parameters *param;
    struct statistic *stat;

    struct timespec trstat, trestat;

    time_t rawtime;
  struct tm * timeinfo;


    union semun arg;
    struct sembuf sops[5];

    struct sigaction sa, sa_old, sia;
    sigset_t my_mask;

    sa.sa_handler = &master_handler;
    sa.sa_flags = 0;
    sigfillset(&my_mask);
    sa.sa_mask = my_mask;

    sia.sa_handler = SIG_IGN;
    sia.sa_flags = 0;
    sigfillset(&my_mask);
    sia.sa_mask = my_mask;
    
    
    sigaction(SIGALRM, &sa, NULL);
    sigaction(SIGINT, &sia, NULL);
    
    
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
    if((param = (struct parameters *) shmat(shm_par, NULL, SHM_RDONLY)) == (void *) -1){
        ERROR_EXIT
    }
    if((stat = (struct statistic *) shmat(shm_stat, NULL, 0)) == (void *) -1){
        ERROR_EXIT
    }
    
    /* Creazione dei semafori */
    if((semid = semget(SEMKEY, 4, IPC_CREAT | 0666)) == -1){
        ERROR_EXIT
    }
    

    /* crea la coda di messaggi per le richieste dei taxi */
    if((qid = msgget(MSGKEY, IPC_CREAT | 0666)) == -1){
        ERROR_EXIT
    }

    /* ----- caricamento parametri, generazione mappa e posizionamento holes ----- */
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

    /* inizializzazione semafori*/
    if((arg.array = (unsigned short *)malloc(sizeof(unsigned short)*4)) == NULL ){
        ERROR_EXIT
    }
    arg.array[SEM_MASTER] = 1;
    arg.array[SEM_SOURCE] = param->so_source;
    arg.array[SEM_TAXI] = param->so_taxi;
    arg.array[SEM_START] = 0;
    if(semctl(semid, 0, SETALL, arg.array) == -1){
        ERROR_EXIT
    }
    free(arg.array);

    init_stat(stat);   
    
    /* creazione dei processi source */
    for(i = 0; i < param->so_source; i++){
        switch (pid = fork()){
            case -1:
                ERROR_EXIT

            case 0: /* ----- codice figlio ----- */
                /* posiziona source e le avvia */
                if(execl("source/source", "source", (char *) NULL) == -1){
                    ERROR_EXIT
                }                  
        }
    }
    
    switch(fork()){
        case -1:
            ERROR_EXIT
        case 0:
            /* ogni 5 secondi stampa lo stato delle celle */
            sops[0].sem_num = SEM_START;
            sops[0].sem_op = -1;
            sops[0].sem_flg = 0;
            if(semop(semid, sops, 1) == -1){
                ERROR_EXIT
            }

            trstat.tv_sec = 5;
            trstat.tv_nsec = 0;
            trestat.tv_sec = 0;
            trestat.tv_sec = 0;

            while(t){
                print_status_cells(city_map);

                sigprocmask(SIG_BLOCK, &my_mask, NULL);
                if(nanosleep(&trstat, &trestat) == -1){
                    ERROR_EXIT
                }
                sigprocmask(SIG_UNBLOCK, &my_mask, NULL);

                sigprocmask(SIG_BLOCK, &my_mask, NULL);
                sops[0].sem_num = SEM_MASTER;
                sops[0].sem_op = -1;
                sops[0].sem_flg = 0;

                if(semop(semid, sops, 1) == -1){
                    ERROR_EXIT
                }

                /* ogni 5 secondi stampa lo stato di occupazione delle celle */
                print_status_cells(city_map);
                 time ( &rawtime );
            timeinfo = localtime ( &rawtime );
                 printf ( "Current local time and date: %s", asctime (timeinfo) );

                sops[0].sem_num = SEM_MASTER;
                sops[0].sem_op = 1;
                sops[0].sem_flg = 0;
                if(semop(semid, sops, 1) == -1){
                    ERROR_EXIT
                }
                sigprocmask(SIG_UNBLOCK, &my_mask, NULL);
                
            }
            exit(EXIT_SUCCESS);
    }
    
    sigaction(SIGCHLD, &sa, &sa_old);
    

    /* creazione dei processi taxi */
    for(i = 0; i < param->so_taxi; i++){
        switch(fork()){
            case -1:
                ERROR_EXIT
            case 0: /* ----- codice figlio ------ */
                /* posiziona i taxi e li avvia */
                if(execl("taxi/taxi", "taxi", (char*) NULL) == -1){
                    ERROR_EXIT
                }
        }
    }

    printf(CGREEN"MASTER %ld provvedo a sbloccare tutto"CDEFAULT"\n", (long)getpid());
    
    /*print_map(city_map);*/

    /* la simulazione parte dopo che sia taxi sia source sono stati creati e inizializzati */
    /* aspetta che il semaforo master diventi 0, lo incrementa per far partire la simulazione e incrementa il semaforo TAXI */
    
    sops[0].sem_num = SEM_SOURCE;
    sops[0].sem_op = 0;                 /* tutti i processi source si sono posizionati */
    sops[0].sem_flg = 0;
    
    sops[1].sem_num = SEM_TAXI;
    sops[1].sem_op = 0;                 /* tutti i processi taxi si sono posizionati */
    sops[1].sem_flg = 0;

    /*sops[2].sem_num = SEM_SOURCE;
    sops[2].sem_op = param->so_source;
    sops[2].sem_flg = 0;

    sops[3].sem_num = SEM_TAXI;
    sops[3].sem_op = param->so_taxi;
    sops[3].sem_flg = 0;*/

    sops[2].sem_num = SEM_START;
    sops[2].sem_op = param->so_taxi + param->so_source + 1;
    sops[2].sem_flg = 0;
    if(semop(semid, sops, 3) == -1){
        ERROR_EXIT
    }
    printf(CRED"tutti processi pronti"CDEFAULT"\n");
    
    /*sops[0].sem_num = SEM_START;
    sops[0].sem_op = param->so_source;
    sops[0].sem_flg = 0;
    if(semop(semid, sops, 1) == -1){
        ERROR_EXIT
    }
*/
    sigaction(SIGTERM, &sa, NULL);
    printf(CGREEN"setto alarm a %d"CDEFAULT"\n", param->so_duration);

    alarm(param->so_duration);

    

    while(t);
    sigaction(SIGCHLD, &sa_old, &sa);
    printf("ORA PROCEDO\n");
    
    /*printf("Richieste totali generate: %d\n", stat->n_request);*/

    
    /* stampa la mappa evidenziando HOLE, SOURCE e TOP_CELLS*/
    for(i = 0; i < param->so_source + param->so_taxi + 1; i++){
        printf("terminato %d\n",waitpid(0, &status, 0));
        /*wait(&status);*/
    }
    printf("processi terminati i %d\n", i);
    printf("numero richieste %d\n", stat->n_request);
    print_map(city_map);

    printf("master semmaster %d\n", semctl(semid, SEM_MASTER, GETVAL));
    printf("master semsource %d\n", semctl(semid, SEM_SOURCE, GETVAL));
    printf("master semtaxi %d\n", semctl(semid, SEM_TAXI, GETVAL));
    /*for(;;){
        print_status_cells(city_map);
        sleep(1);
    }*/
    
    
    /*print_status_cells(city_map);*/
    
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
         printf ( "Current local time and date: %s", asctime (timeinfo) );

    /* rimuove la coda di messaggi */
    if(msgctl(qid, IPC_RMID, NULL)){
        ERROR_EXIT
    }

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

    /* rimozione dei semafori */
    if(semctl(semid,0, IPC_RMID) == -1){
        ERROR_EXIT
    }

    exit(EXIT_SUCCESS);

}
