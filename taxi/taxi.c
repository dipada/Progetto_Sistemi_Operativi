#include "../master/master.h"

int main(int argc, char** argv){
    struct timespec tim, rem;
    

    map *city_map;
    struct parameters *param;
    struct statistic *stat;
    taxi_t taxi;

    int shm_map, shm_par, shm_stat, semid, qid , source_pos, i, j, x;
    
    union semun arg;
    /*struct sembuf sops[3];*/
    
    struct request_queue q;
    /*pid_t pid;*/
    tim.tv_sec = 0;
    tim.tv_nsec = 999999999;

    

    /* Creazione shm*/
    if((shm_map = shmget(SHMKEY_MAP, sizeof(map), 0)) == -1){
        ERROR_EXIT
    }
    if((shm_par = shmget(SHMKEY_PAR, sizeof(struct parameters), 0)) == -1){
        ERROR_EXIT
    }
    if((shm_stat = shmget(SHMKEY_STAT, sizeof(struct statistic), 0)) == -1){
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
    if((semid = semget(SEMKEY, 3, 0)) == -1){
        ERROR_EXIT
    }
    /* inizializzazione semafori*/
    if((arg.array = (unsigned short *)malloc(sizeof(unsigned short)*3)) == NULL ){
        ERROR_EXIT
    }
    /* crea la coda di messaggi per le richieste dei taxi */
    if((qid = msgget(IPC_PRIVATE, IPC_CREAT | 0666)) == -1){
        ERROR_EXIT
    }

    
    /* inizializza struttura statistiche */
    initialize_stat(stat);    
    
    /* crea SO_SOURCE processi dedicati che si legano ad una cella della mappa  */
    for(i = 0; i < param->so_source; i++){
        switch(fork()){
            case -1:
                ERROR_EXIT
            case 0: /* ----- codice figlio -----  */
                /* processo SOURCE si associa ad una cella libera che non sia HOLE */
                source_pos = place_source(city_map, 1, SO_WIDTH*SO_HEIGHT);
                
                /* richiesta di accesso alla sezione critica */
               


                /* ----- SEZIONE CRITICA ----- */
                printf("SOURCE HA PRESO IL CONTROLLOOOO\n");

                /* genera richieste taxi con un intervallo variabile*/
                tim.tv_nsec = get_random(1, 999999999); /* genera una richiesta in un tempo variabile tra 1 nsec - 1 sec */
                if(nanosleep(&tim, &rem) == -1){
                    ERROR_EXIT
                }                    
                
                /* preparazione della richiesta */
                q.start_cell = (long)getpid();
                /* genera una posizione che sia una cella diversa dall'attuale e non sia hole */
                q.aim_cell = get_aim_cell(city_map, source_pos);
                
                if(msgsnd(qid, &q, sizeof(struct request_queue) - sizeof(long), 0) == -1){
                    ERROR_EXIT
                }
                
                printf("source %ld posizione %d, inserita cella di destinazione %ld \n", (long)getpid(), source_pos, q.aim_cell);

                /* registrazione dell'avvenuta creazione della richiesta */
                stat->n_request +=1;
                

                exit(EXIT_SUCCESS);

            default:
            wait(NULL);
            
        }
    }
    
    /* crea SO_TAXI processi */
    for(j = 0; j < param->so_taxi; j++){
        switch(fork()){
            case -1:
                ERROR_EXIT
            case 0:
                /* i taxi si posizionano casualmente */
                place_taxi(city_map, &taxi);
                


                /* ----- SEZIONE CRITICA ----- */
                printf("TAXI HA PRESO IL CONTROLLOOOO\n");

                /* TODO SEMAFORI e TIMER TAXI*/
                
                /* se il taxi si trova in una cella SOURCE con richieste attive, preleva quella richiesta */
                if(msgrcv(qid, &q, sizeof(struct request_queue)- sizeof(long), taxi.pid_cell_taxi, IPC_NOWAIT) == -1){
                    if(errno != ENOMSG){
                        ERROR_EXIT
                    }
                }else{
                    printf("taxi %ld, prelevata cella di destinazione %ld\n", (long)getpid(), q.aim_cell);
                }
                /* ogni taxi preleva le richieste delle sole celle in cui si trova */
                
                exit(EXIT_SUCCESS);
            default:
                wait(NULL);
        }
    }

    /* sblocco i semafori SOURCE e TAXI per far partire la simulazione */
    x = 0;
    while(x != 2){
        if(i == param->so_source){
            /* sblocca semaforo SOURCE*/
            x++;
        }
        if(j == param->so_taxi){
            /* sblocca semaforo TAXI */
            x++;
        }
    }
    printf("procedo a sbloccare\n");
    
    /* sblocca il processo master per far avviare la simulazione */
    




    printf("taxi %d, j %d, source %d, i %d,\n", param->so_taxi,j, param->so_source, i);
    for(i = 0; i < SO_WIDTH*SO_HEIGHT; i++){
        if(city_map->m_cell[i].n_taxi_here > 0)
            printf("ncella %d, source %ld, ho n taxi %d\n",i, city_map->m_cell[i].pid_source, city_map->m_cell[i].n_taxi_here);
    }    
    printf("Richieste totali generate: %d\n", stat->n_request);
    
    

    /* rimuove la coda di messaggi */
    if(msgctl(qid, IPC_RMID, NULL)){
        ERROR_EXIT
    }

    /* detach SHM */
    if(shmdt(city_map) == -1){
        ERROR_EXIT
    }
    if(shmdt(param) == -1){
        ERROR_EXIT
    }
    if(shmdt(stat) == -1){
        ERROR_EXIT
    }


    exit(EXIT_SUCCESS);
}