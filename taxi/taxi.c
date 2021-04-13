#include "../master/master.h"
#include "taxi.h"

static int t = 1;

map *city_map;
struct statistic *stat;

int qid, semid, source_pos;

struct request_queue q;

struct sembuf sops[3];

void taxi_handler(int sig){
    if(sig == SIGTERM){
        /*printf("%s Ricevuto SIGTERM PID %ld\n", __FILE__, (long)getpid());*/
        t = 0;
    }
    if(sig == SIGINT){ /* usato per generare richieste */

        /* preparazione della richiesta */
        q.start_cell = (long)getpid();
        /* genera una posizione che sia una cella diversa dall'attuale e non sia hole */
        q.aim_cell = get_aim_cell(city_map, source_pos);
        if(msgsnd(qid, &q, sizeof(struct request_queue) - sizeof(long), IPC_NOWAIT) == -1){
            if(errno == EAGAIN){
                fprintf(stderr, "\nSource[%d]>> Request queue is full. Can't accept your request\n", source_pos);
            }else{
                ERROR_EXIT
            }
        }else{
            printf("\nSource[%d]>> Request accepted. Start: %d Dest: %ld\n",source_pos, source_pos, q.aim_cell);
        
            sops[0].sem_num = SEM_MASTER;
            sops[0].sem_op = -1;
            sops[0].sem_flg = 0;
            if(semop(semid, sops, 1) == -1){
                ERROR_EXIT
            }               
            
            /* aggiorno il numero di richieste generato */
            stat->n_request +=1;
                    
            sops[0].sem_num = SEM_MASTER;
            sops[0].sem_op = 1;
            sops[0].sem_flg = 0;

            /*sops[1].sem_num = SEM_SOURCE;
            sops[1].sem_op = 1;
            sops[1].sem_flg = 0;*/

            if(semop(semid, sops, 1) == -1){
                ERROR_EXIT
            }
        }
    }
}


int main(int argc, char** argv){
    
    
    struct parameters *param;
    taxi_t taxi;

    int shm_map, shm_par, shm_stat, i, j, x;

    union semun arg;
    
    
    
    /*pid_t pid;*/

    struct sigaction sa;
    sigset_t my_mask;
    
    /* per nanosleep */
    struct timespec treq, trem, tsop;
    treq.tv_sec = 0;
    treq.tv_nsec = 0;
    trem.tv_sec = 0;
    trem.tv_nsec = 0;
    tsop.tv_sec = 0;
    tsop.tv_nsec = 0;
    

    sa.sa_handler = &taxi_handler;
    sa.sa_flags = 0;
    sigfillset(&my_mask);
    sa.sa_mask = my_mask;
    
    sigaction(SIGTERM, &sa, NULL);
    
    

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
                
                /* processo SOURCE si associa ad una cella libera che non sia HOLE e registra source */
                source_pos = place_source(city_map);
                sigaction(SIGINT, &sa, NULL);
                
                printf("source pid %ld\n", (long)getpid());
                while(t){
                    
                    /* in attesa del master che autorizza l'avvio della simulazione */
                    sops[0].sem_num = SEM_SOURCE;
                    sops[0].sem_op = -1;
                    sops[0].sem_flg = 0;
    
                    /* incremento il semaforo source e decremento master 
                    sops[1].sem_num = SEM_MASTER;
                    sops[1].sem_op = -1;
                    sops[1].sem_flg = 0;*/

                    if(semop(semid, sops, 1) == -1){
                        if(errno == EINTR){
                            exit(EXIT_FAILURE);
                        }else{
                            ERROR_EXIT
                        }
                    }
                    
                    /* genera richieste taxi con un intervallo variabile tra 1 nsec - 1 sec */
                    sigprocmask(SIG_BLOCK, &my_mask, NULL);
                    treq.tv_nsec = get_random(1, 999999999);
                    if(nanosleep(&treq, &trem) == -1){
                        if(errno == EINTR){
                            exit(EXIT_FAILURE);
                        }else{
                            ERROR_EXIT
                        }
                    }                
                    sigprocmask(SIG_UNBLOCK, &my_mask, NULL);
                    
                    /* preparazione della richiesta */
                    q.start_cell = (long)getpid();
                    /* genera una posizione che sia una cella diversa dall'attuale e non sia hole */
                    q.aim_cell = get_aim_cell(city_map, source_pos);
                
                    if(msgsnd(qid, &q, sizeof(struct request_queue) - sizeof(long), 0) == -1){
                        if(errno == EINTR){
                            exit(EXIT_FAILURE);
                        }else{
                            ERROR_EXIT
                        }
                    }
                
                    
                    sigprocmask(SIG_BLOCK, &my_mask, NULL);
                    sops[0].sem_num = SEM_MASTER;
                    sops[0].sem_op = -1;
                    sops[0].sem_flg = 0;
                    if(semop(semid, sops, 1) == -1){
                        if(errno == EINTR){
                            exit(EXIT_FAILURE);
                        }else{
                            ERROR_EXIT
                        }
                    
                    }
                    

                    /* ----- SEZIONE CRITICA ----- */
                
                    /* registrazione dell'avvenuta creazione della richiesta */                
                    
                    stat->n_request +=1;      

                    printf("Source %ld richiesta partenza %d di arrivo a %ld registrata\n", (long)getpid(), source_pos, q.aim_cell);
                    /* ----- FINE SEZIONE CRITITCA ----- */
                    
                    sops[0].sem_num = SEM_MASTER;
                    sops[0].sem_op = 1;
                    sops[0].sem_flg = 0;

                    /*sops[1].sem_num = SEM_SOURCE;
                    sops[1].sem_op = 1;
                    sops[1].sem_flg = 0;*/

                    if(semop(semid, sops, 1) == -1){
                        if(errno == EINTR){
                            exit(EXIT_FAILURE);
                        }else{
                            ERROR_EXIT
                        }
                    }
                    sigprocmask(SIG_UNBLOCK, &my_mask, NULL);
                    
                    sops[0].sem_num = SEM_SOURCE;
                    sops[0].sem_op = 1;
                    sops[0].sem_flg = 0;

                    if(semop(semid, sops, 1) == -1){
                        if(errno == EINTR){
                            exit(EXIT_FAILURE);
                        }else{
                            ERROR_EXIT
                        }
                    }
                }

                exit(EXIT_SUCCESS);
        }
    }
    
    /* crea SO_TAXI processi */
    for(j = 0; j < param->so_taxi; j++){
        switch(fork()){
            case -1:
                ERROR_EXIT
            case 0:
                /* i taxi si posizionano casualmente */
                
                
                tsop.tv_sec = param->so_timeout;
                
                place_taxi(city_map, &taxi);
                
                while(t){
                    printf("Taxi pid %ld, è in cella %ld num %d\n", (long)getpid(), taxi.pid_cell_taxi, taxi.where_taxi);    
                    
                    sops[0].sem_num = SEM_TAXI;
                    sops[0].sem_op = -1;
                    sops[0].sem_flg = 0;
                    if(semop(semid, sops, 1) == -1){
                        if(errno == EINTR){
                            exit(EXIT_FAILURE);
                        }else{
                            ERROR_EXIT
                        }
                    }
                    
                    printf("taxi %ld suorcecell = %d\n", (long)getpid(), city_map->m_cell[taxi.where_taxi].is_source);
                    /*printf("taxi %ld !suorcecell = %d\n", (long)getpid(),!city_map->m_cell[taxi.where_taxi].is_source);*/
                    
                 /*   if(!city_map->m_cell[taxi.where_taxi].is_source){
                          verificare che la cella in cui è il taxi sia source altrimenti raggiunge una cella source 
                        
                        source_pos = search_source(city_map, taxi.where_taxi);
                        printf("taxi %ld è qui %d, va alla source %d\n", (long)getpid(), taxi.where_taxi, source_pos);
                        while(source_pos != taxi.where_taxi){
                            
                            sops[0].sem_num = SEM_MASTER;
                            sops[0].sem_op = -1;
                            sops[0].sem_flg = 0;

                            if(semtimedop(semid, sops, 1, &tsop) == -1){
                                if(errno == EAGAIN){ 
                                    printf("taxi %ld POS %d EAGAIN\n", (long)getpid(), taxi.where_taxi);
                                    city_map->m_cell[taxi.where_taxi].n_taxi_here -= 1;
                                    
                                    sops[0].sem_num = SEM_TAXI;
                                    sops[0].sem_op = 1;
                                    sops[0].sem_flg = 0;
                                    if(semop(semid, sops, 1) == -1){
                                        ERROR_EXIT
                                    }
                                    exit(EXIT_FAILURE);
                                }
                                ERROR_EXIT
                            }

                            
                            go_cell(city_map, &taxi, source_pos);
                            treq.tv_nsec = city_map->m_cell[taxi.where_taxi].cross_time;
                            nanosleep(&treq, &trem);
                            
                            
                            sops[0].sem_num = SEM_MASTER;
                            sops[0].sem_op = 1;
                            sops[0].sem_flg = 0;
                            if(semop(semid, sops, 1) == -1){
                                ERROR_EXIT
                            }
                            
                            
                        }
                        printf("arrivato alla source %d\n", taxi.where_taxi);
                    }
                    */

                    
                    
                    /*printf("taxi %ld aspetto master\n", (long)getpid());*/
                    
                    /* preleva la richiesta */
                    if(msgrcv(qid, &q, (sizeof(struct request_queue) - sizeof(long)), taxi.pid_cell_taxi, 0) == -1){
                        
                        /*if(errno == EINTR){
                            exit(EXIT_FAILURE);
                        }*/
                        ERROR_EXIT
                        
                    }
                    

                    printf("taxi %ld curp %d arrivo %ld\n",(long)getpid(), taxi.where_taxi, q.aim_cell);
                    
                    /* esegue la richiesta */
                    
                    while(q.aim_cell != taxi.where_taxi){
                        
                        sops[0].sem_num = SEM_MASTER;
                        sops[0].sem_op = -1;
                        sops[0].sem_flg = 0;
                        if(semtimedop(semid, sops, 1, &tsop) == -1){
                            if(errno == EAGAIN){ 
                                exit(EXIT_FAILURE);
                            }else{
                                ERROR_EXIT
                            }
                        }

                        go_cell(city_map, &taxi, q.aim_cell);

                        printf("taxi %ld sono dentro\n", (long)getpid());
                        treq.tv_nsec = city_map->m_cell[taxi.where_taxi].cross_time;
                        nanosleep(&treq, &trem);

                        

                        sops[0].sem_num = SEM_MASTER;
                        sops[0].sem_op = 1;
                        sops[0].sem_flg = 0;

                        if(semop(semid, sops, 1) == -1){
                            if(errno == EINTR){
                                exit(EXIT_FAILURE);
                            }else{
                                ERROR_EXIT
                            }
                        }
                        
                        
                    }
                    stat->success_req += 1;
                    city_map->m_cell[taxi.where_taxi].n_taxi_here -= 1;
                    printf("taxi %ld arrivato a destinazione a cella %d\n",(long)getpid(), taxi.where_taxi);

                    sops[1].sem_num = SEM_TAXI;
                    sops[1].sem_op = 1;
                    sops[1].sem_flg = 0;

                    if(semop(semid, sops, 1) == -1){
                        if(errno == EINTR){
                            exit(EXIT_FAILURE);
                        }else{
                            ERROR_EXIT
                        }
                    }
                }

                exit(EXIT_SUCCESS);
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
    
    for(i = 0; i < param->so_taxi + param->so_source; i++){
        sigprocmask(SIG_BLOCK, &my_mask, NULL);
        waitpid(0,NULL,0);
        /*printf("terminato %d\n", waitpid(0,NULL,0));*/
        sigprocmask(SIG_UNBLOCK, &my_mask, NULL);
    }    
    
    
    
    printf("n request %d, SUCCESS REQUEST = %d ABORTED = %d\n",stat->n_request, stat->success_req, stat->aborted_req);

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