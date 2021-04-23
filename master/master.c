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

struct sembuf sops[5];
int semid;

int main(int argc, char **argv){

    int status, shm_map, shm_par, shm_stat, qid, i;
    pid_t pid;
    map *city_map;
    struct parameters *param;
    struct statistic *stat;

    struct timespec trstat, trestat;

    union semun arg;
    
    struct sigaction sa, sia;
    sigset_t my_mask;

    sa.sa_handler = &master_handler;
    sa.sa_flags = SA_NODEFER;
    sigemptyset(&my_mask);
    sa.sa_mask = my_mask;

    sia.sa_handler = SIG_IGN;
    sia.sa_flags = 0;
    sigfillset(&my_mask);
    sia.sa_mask = my_mask;
    
    
    sigaction(SIGALRM, &sa, NULL);
    sigaction(SIGINT, &sia, NULL);
    
    
    /* Creazione SHM */
    if((shm_map = shmget(SHMKEY_MAP, sizeof(map), IPC_CREAT | 0644)) == -1){
        ERROR_EXIT
    }
    if((shm_par = shmget(SHMKEY_PAR, sizeof(struct parameters), IPC_CREAT | 0644)) == -1){
        ERROR_EXIT
    }
    if((shm_stat = shmget(SHMKEY_STAT, sizeof(struct statistic), IPC_CREAT | 0644)) == -1){
        ERROR_EXIT
    }

    /* attach delle shm */
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
    if((semid = semget(SEMKEY, 5, IPC_CREAT | 0666)) == -1){
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
            if(WIFEXITED(status)){
                if(WEXITSTATUS(status)){ /* analizza lo status di uscita */
                    fprintf(stderr,"[%s]: Exiting due to error map...\n", __FILE__);
                    free_all();
                    exit(EXIT_FAILURE);
                }  
            }
    }

    /* inizializzazione semafori*/
    if((arg.array = (unsigned short *)malloc(sizeof(unsigned short)*5)) == NULL ){
        ERROR_EXIT
    }
    arg.array[SEM_MASTER] = 1;
    arg.array[SEM_SOURCE] = param->so_source;
    arg.array[SEM_TAXI] = param->so_taxi;
    arg.array[SEM_START] = 0;
    arg.array[SEM_ST] = 0;

    if(semctl(semid, 0, SETALL, arg.array) == -1){
        ERROR_EXIT
    }
    free(arg.array);

    /* inizializza le statistiche */
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
            /* ogni second0 stampa lo stato di occupazione delle celle */
            sops[0].sem_num = SEM_START;
            sops[0].sem_op = -1;
            sops[0].sem_flg = 0;
            if(semop(semid, sops, 1) == -1){
                ERROR_EXIT
            }

            trstat.tv_sec = 1;
            trstat.tv_nsec = 0;
            trestat.tv_sec = 0;
            trestat.tv_sec = 0;

            while(t){
                /*print_status_cells(city_map);*/

                sigprocmask(SIG_BLOCK, &my_mask, NULL);
                
                sops[0].sem_num = SEM_MASTER;
                sops[0].sem_op = -1;
                sops[0].sem_flg = 0;

                if(semop(semid, sops, 1) == -1){
                    ERROR_EXIT
                }

                /* ogni 5 secondi stampa lo stato di occupazione delle celle */
                print_status_cells(city_map);

                sops[0].sem_num = SEM_MASTER;
                sops[0].sem_op = 1;
                sops[0].sem_flg = 0;
                if(semop(semid, sops, 1) == -1){
                    ERROR_EXIT
                }

                if(nanosleep(&trstat, &trestat) == -1){
                    ERROR_EXIT
                }
                sigprocmask(SIG_UNBLOCK, &my_mask, NULL);
                
            }
            exit(EXIT_SUCCESS);
    }
    
    sigaction(SIGUSR1, &sa, NULL);
    
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
    
    printf(CYELLOW"\t\tWelcome to TaxiCab Game.\n"CDEFAULT);
    printf("Press "CYELLOW"Ctrl^C"CDEFAULT" for let all source make new request\n");
    printf("Game will start in:\n");
    for(i = 5; i > 0; i--){
        printf("\t%d", i);
        fflush(stdout);
        sleep(1);
        printf("\r");
    }
    printf(CGREEN"\tStarted"CDEFAULT"\n\n");

    /* la simulazione parte dopo che sia taxi sia source sono stati creati e inizializzati */
    /* aspetta che il semaforo master diventi 0, lo incrementa per far partire la simulazione e incrementa il semaforo TAXI */
    
    sops[0].sem_num = SEM_SOURCE;
    sops[0].sem_op = 0;                 /* tutti i processi source si sono posizionati */
    sops[0].sem_flg = 0;
    
    sops[1].sem_num = SEM_TAXI;
    sops[1].sem_op = 0;                 /* tutti i processi taxi si sono posizionati */
    sops[1].sem_flg = 0;

    sops[2].sem_num = SEM_START;
    sops[2].sem_op = param->so_taxi + param->so_source + 1;
    sops[2].sem_flg = 0;
    
    sops[3].sem_num = SEM_ST;
    sops[3].sem_op = 1;
    sops[3].sem_flg = 0;

    if(semop(semid, sops, 4) == -1){
        ERROR_EXIT
    }
    
    sigaction(SIGTERM, &sa, NULL);

    alarm(param->so_duration);    

    while(t);
    
    kill(0, SIGTERM);
    
    /* terminazione */
    for(i = 0; i < param->so_source + param->so_taxi + 1; i++){
        sigprocmask(SIG_BLOCK, &my_mask, NULL);
        wait(NULL);
        sigprocmask(SIG_UNBLOCK, &my_mask, NULL);
    }

    /* stampa la mappa evidenziando HOLE, SOURCE e TOP_CELLS*/
    printf("\n");
    print_map(city_map, param->so_top_cells);

    /* stampa statistiche */   
    printf("\n\nTotal request: [%d] Success: ["CGREEN"%d"CDEFAULT"] Aborted:["CRED"%d"CDEFAULT"] Outstanding: ["CYELLOW"%d"CDEFAULT"]\n", \
        stat->n_request, stat->success_req, stat->aborted_req, stat->outstanding_req + (stat->n_request-stat->aborted_req-stat->success_req));
    
    printf("Taxi that cross more cell [%ld], number of cell crossed [%d]\n",stat->pid_hcells_taxi, stat->high_ncells_crossed);
    printf("Taxi that has took most time to complete a request: [%ld], time: %ld seconds\n", stat->pid_htime_taxi, stat->high_time);
    printf("Taxi that has collect more requests: [%ld], number of requests collected: [%d]\n", stat->pid_hreq_taxi, stat->n_high_req);

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

void master_handler(int sig){
    if(sig == SIGALRM){ /* scaduto il tempo della simulazione */
        kill(0, SIGTERM); 
    }

    if(sig == SIGTERM){
        t = 0;
    }

    if(sig == SIGUSR1){ /* un taxi è terminato */
        wait(NULL);
        if(t){
            switch(fork()){
                case -1:
                    ERROR_EXIT
                case 0:
                    if(execl("taxi/taxi", "taxi", (char*) NULL) == -1){
                        ERROR_EXIT
                    }
            }
        }
    }
}