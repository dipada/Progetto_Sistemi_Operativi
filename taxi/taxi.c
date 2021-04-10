#include "../master/master.h"
#include "taxi.h"

int main(int argc, char** argv){
    
    map *city_map;
    struct parameters *param;
    struct statistic *stat;
    taxi_t taxi;

    int shm_map, shm_par, shm_stat, semid, qid , source_pos, i, j, x;

    union semun arg;
    struct sembuf sops[3];
    /*struct sembuf sops[3];*/
    
    struct request_queue q;
    /*pid_t pid;*/

    /* per nanosleep */
    struct timespec treq, trem, tsop;
    treq.tv_sec = 0;
    treq.tv_nsec = 0;
    trem.tv_sec = 0;
    trem.tv_nsec = 0;
    tsop.tv_sec = 0;
    tsop.tv_nsec = 500;
    

    

    /* Recupero id shm*/
    if((shm_map = shmget(SHMKEY_MAP, sizeof(map), 0)) == -1){
        ERROR_EXIT
    }
    if((shm_par = shmget(SHMKEY_PAR, sizeof(struct parameters), SHM_RDONLY)) == -1){
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
                
                
                /* in attesa del master che autorizza l'avvio della simulazione */
                sops[0].sem_num = SEM_SOURCE;
                sops[0].sem_op = -1;
                sops[0].sem_flg = 0;
   
                /* incremento il semaforo source e decremento master */
                sops[1].sem_num = SEM_MASTER;
                sops[1].sem_op = -1;
                sops[1].sem_flg = 0;

                if(semop(semid, sops, 2) == -1){
                    ERROR_EXIT
                }
                                
                /* genera richieste taxi con un intervallo variabile tra 1 nsec - 1 sec */
                treq.tv_nsec = get_random(1, 1);
                if(nanosleep(&treq, &trem) == -1){
                    ERROR_EXIT
                }                    
                
                /* preparazione della richiesta */
                q.start_cell = (long)getpid();
                /* genera una posizione che sia una cella diversa dall'attuale e non sia hole */
                q.aim_cell = get_aim_cell(city_map, source_pos);
                
                if(msgsnd(qid, &q, sizeof(struct request_queue) - sizeof(long), 0) == -1){
                    ERROR_EXIT
                }
                printf("SOURCE %ld richiesta effettuata. Partenza da %d arrivo %ld\n", (long)getpid(), source_pos, q.aim_cell);

                

                
                
                /* ----- SEZIONE CRITICA ----- */
                
                /* registrazione dell'avvenuta creazione della richiesta */
                
                
                    

                stat->n_request +=1;

                
                

                sops[0].sem_num = SEM_MASTER;
                sops[0].sem_op = 1;
                sops[0].sem_flg = 0;

                sops[1].sem_num = SEM_SOURCE;
                sops[1].sem_op = 1;
                sops[1].sem_flg = 0;

                if(semop(semid, sops, 2) == -1){
                    ERROR_EXIT
                }
                
                /* ----- FINE SEZIONE CRITITCA ----- */

                exit(EXIT_SUCCESS);

            /*default:
            wait(NULL);*/
            
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
                
                sleep(1);
                /* in attesa del master che autorizza l'avvio della simulazione */
                

                
                
                
                

                /* ----- SEZIONE CRITICA ----- */
                /*printf("TAXI %ld\n",(long)getpid());*/

                /* TODO SEMAFORI e TIMER TAXI*/
                
                /* ricerca di una richiesta da eseguire */
                /* se il taxi si trova in una cella SOURCE con richieste attive, preleva quella richiesta */
                if(msgrcv(qid, &q, sizeof(struct request_queue)- sizeof(long), taxi.pid_cell_taxi, IPC_NOWAIT) == -1){
                    if(errno != ENOMSG){
                        ERROR_EXIT
                    }else{
                        printf("taxi %ld era in %d, uscito\n", (long)getpid(), taxi.where_taxi);
                        city_map->m_cell[taxi.where_taxi].n_taxi_here -= 1;
                        exit(EXIT_SUCCESS);
                    }
                }else{
                    /*printf("taxi %ld, posizione %d, prelevata cella di destinazione %ld\n", (long)getpid(), taxi.where_taxi, q.aim_cell);*/

                    
                       
                    
                    

                    printf("taxi %ld, cur_p %d, goal_pos %ld, j %d\n", (long)getpid(), taxi.where_taxi, q.aim_cell, j);

                    
                    while(q.aim_cell != taxi.where_taxi){
                        
                        printf("%d sem taxi vale %d sem master vale %d\n", j, semctl(semid, SEM_TAXI, GETVAL), semctl(semid, SEM_MASTER, GETVAL));
                        sops[0].sem_num = SEM_TAXI;
                        sops[0].sem_op = -1;
                        sops[0].sem_flg = 0;

                        sops[1].sem_num = SEM_MASTER;
                        sops[1].sem_op = -1;
                        sops[1].sem_flg = 0;

                        if(semtimedop(semid, sops, 2, &tsop) == -1){
                                city_map->m_cell[taxi.where_taxi].n_taxi_here -= 1;
                                ERROR_EXIT
                        }
                    
                        go_cell(city_map, &taxi, q.aim_cell);
                        
                        
                        

                        sops[0].sem_num = SEM_TAXI;
                        sops[0].sem_op = 1;
                        sops[0].sem_flg = 0;

                        sops[1].sem_num = SEM_MASTER;
                        sops[1].sem_op = 1;
                        sops[1].sem_flg = 0;

                        if(semtimedop(semid, sops, 2, &tsop) == -1){
                            if(errno == EAGAIN){
                                printf("taxi %ld tempo sops scaduto release. Killed\n", (long)getpid());
                                exit(EXIT_SUCCESS);
                            }
                            ERROR_EXIT
                        }

                        printf("%d sem taxi vale %d sem master vale %d\n", j, semctl(semid, SEM_TAXI, GETVAL), semctl(semid, SEM_MASTER, GETVAL));                  
                    }
                    printf("Arrivato a destinazione. Il taxi %ld si trova qui %d\n", (long)getpid(), taxi.where_taxi);
                    city_map->m_cell[taxi.where_taxi].n_taxi_here -= 1;
                    exit(EXIT_SUCCESS);
                }
                /* ogni taxi preleva le richieste delle sole celle in cui si trova */
                
                
                
                
                
            /*default:
                wait(NULL);*/
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
    sops[0].sem_num = SEM_MASTER;
    sops[0].sem_op = -1;
    sops[0].sem_flg = 0;

    if(semop(semid, sops, 1) == -1){
        ERROR_EXIT
    }
    printf("Sbloccaggio avvenuto\n");
    
    for(i = 0; i < param->so_taxi; i++){
        wait(NULL);
    }    
    for(i = 0; i < param->so_source; i++){
        wait(NULL);
    }

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