#include "../master/master.h"
#include "taxi.h"

static int t = 1;

map* city_map;
struct statistic* stat;

taxi_t taxi;

int semid;

struct sembuf sops[3];

void taxi_handler(int sig){
    if(sig == SIGTERM){
        /*printf("%s Ricevuto SIGTERM PID TAXI %ld\n", __FILE__, (long)getpid());*/
        t = 0;
    }

    if(sig == SIGALRM){    /* il taxi non si muove seppur riesce ad accedere al SEM_MASTER */
        
        /*printf("taxi %ld "CYELLOW"uscita alarm"CDEFAULT" taxi %d \n", (long)getpid(), city_map->m_cell[taxi.where_taxi].n_taxi_here);*/
        if(t){
            
            sops[0].sem_num = SEM_START;
            sops[0].sem_op = 1;
            sops[0].sem_flg = 0;

            sops[1].sem_num = SEM_TAXI;
            sops[1].sem_op = 1;
            sops[1].sem_flg = 0;

            if(semop(semid, sops, 2) == -1){
                ERROR_EXIT
            }

            if(kill(getppid(), SIGUSR1) == -1){
                ERROR_EXIT
            }
        }
        city_map->m_cell[taxi.where_taxi].n_taxi_here -= 1;
        exit(EXIT_FAILURE);
        
    }
}

int main(int argc, char** argv){

    int shm_map, shm_par, shm_stat, qid, src_pos, alr_pos;

    struct parameters* param;

    struct request_queue queue;

    struct timespec treq, trem, tsop;

    struct sigaction sa;
    sigset_t my_mask;

    time_t start_time, end_time;

    sa.sa_handler = &taxi_handler;
    sa.sa_flags = 0;
    sigfillset(&my_mask);
    
    sa.sa_mask = my_mask;
    sigaction(SIGTERM, &sa, NULL);
    
    /* recupero ID SHM */
    if((shm_map = shmget(SHMKEY_MAP, sizeof(map), 0)) == -1){
        ERROR_EXIT
    }
    if((shm_par = shmget(SHMKEY_PAR, sizeof(struct parameters), SHM_RDONLY)) == -1){
        ERROR_EXIT
    }
    if((shm_stat = shmget(SHMKEY_STAT, sizeof(struct statistic), 0)) == -1){
        ERROR_EXIT
    }

    /* attach SHM */
    if((city_map = (map *) shmat(shm_map, NULL, 0)) == (void *) -1){
        ERROR_EXIT
    }
    if((param = (struct parameters *) shmat(shm_par, NULL, 0)) == (void *) -1){
        ERROR_EXIT
    }
    if((stat = (struct statistic *) shmat(shm_stat, NULL, 0)) == (void *) -1){
        ERROR_EXIT
    }

    /* ID semafori */
    if((semid = semget(SEMKEY, 5, 0)) == -1){
        ERROR_EXIT
    }

    /* coda di messaggi */
    if((qid = msgget(MSGKEY, 0)) == -1){
        ERROR_EXIT
    }
    
    
    sigaction(SIGALRM, &sa, NULL);
    /*printf("taxi %ld semmaster vale %d\n", (long)getpid(), semctl(semid, SEM_MASTER, GETVAL));*/

    sigprocmask(SIG_BLOCK, &my_mask, NULL);
    sops[0].sem_num = SEM_MASTER;
    sops[0].sem_op = -1;
    sops[0].sem_flg = 0;
    if(semop(semid, sops, 1) == -1){
        ERROR_EXIT
    }
    /* posizionamento casuale del taxi */
    place_taxi(city_map, &taxi);

    sops[0].sem_num = SEM_MASTER;
    sops[0].sem_op = 1;
    sops[0].sem_flg = 0;

    sops[1].sem_num = SEM_TAXI;
    sops[1].sem_op = -1;
    sops[1].sem_flg = 0;
    if(semop(semid, sops, 2) == -1){
        ERROR_EXIT
    }
    sigprocmask(SIG_UNBLOCK, &my_mask, NULL);
    /*printf("taxi %ld posizione %d\n", (long)getpid(), taxi.where_taxi);*/

    /* in attesa dell'autorizzazione dal processo master */
    /* master attende lo 0 di SEM_TAXI */
    sops[0].sem_num = SEM_START;
    sops[0].sem_op = -1;
    sops[0].sem_flg = 0;
    if(semop(semid, sops, 1) == -1){
        ERROR_EXIT
    }
    
    tsop.tv_sec = param->so_timeout;
    tsop.tv_nsec = 0;

    while(t){
        
        /* se non è in una source ci va */
        if(!city_map->m_cell[taxi.where_taxi].is_source){
            src_pos = search_source(city_map, taxi.where_taxi);
            
            /*printf("taxi %ld source localizzata a %d\n", (long)getpid(), src_pos);*/
            alr_pos = taxi.where_taxi;
            alarm(param->so_timeout);
            while(taxi.where_taxi != src_pos && t){
                

                sigprocmask(SIG_BLOCK, &my_mask, NULL);
                sops[0].sem_num = SEM_MASTER;
                sops[0].sem_op = -1;
                sops[0].sem_flg = 0;
                if(semtimedop(semid, sops, 1, &tsop) == -1){
                    if(errno == EAGAIN){
                        if(t){
                            sops[0].sem_num = SEM_START;
                            sops[0].sem_op = 1;
                            sops[0].sem_flg = 0;

                            sops[1].sem_num = SEM_TAXI;
                            sops[1].sem_op = 1;
                            sops[1].sem_flg = 0;

                            if(semop(semid, sops, 2) == -1){
                                ERROR_EXIT
                            }
                            kill(getppid(), SIGUSR1);                            
                        }
                        city_map->m_cell[taxi.where_taxi].n_taxi_here -= 1;
                        /*printf("taxi %ld uscita "CGREEN"SEMTIMEDOP NORMALE"CDEFAULT"\n", (long)getpid());*/
                        exit(EXIT_FAILURE);
                    }
                }
                

                go_cell(city_map, &taxi, src_pos);
                /* il taxi si è mosso annullo l'alarm */
                if(alr_pos != taxi.where_taxi){ 
                    alarm(param->so_timeout);
                    alr_pos = taxi.where_taxi;
                }
                
                sops[0].sem_num = SEM_MASTER;
                sops[0].sem_op = 1;
                sops[0].sem_flg = 0;
                if(semop(semid, sops, 1) == -1){
                    ERROR_EXIT
                }

                treq.tv_sec = 0;
                treq.tv_nsec = city_map->m_cell[taxi.where_taxi].cross_time;
                if(nanosleep(&treq, &trem) == -1){
                    ERROR_EXIT
                }
                
                sigprocmask(SIG_UNBLOCK, &my_mask, NULL);
            }
            alarm(0);
           /*printf("taxi %ld arrivato alla source %d\n", (long)getpid(), taxi.where_taxi);*/
        }

        /* preleva una richiesta dalla coda di messaggi */
        if(take_request(city_map, qid, &taxi, &queue) == -1){
            if(errno != ENOMSG){
                exit(EXIT_FAILURE);
            }
        }else{
            /*printf("taxiwhere %d queue dest %d\n", taxi.where_taxi, queue.dest_cell);*/
            taxi.nreq += 1;

            sigprocmask(SIG_BLOCK, &my_mask, NULL);
            sops[0].sem_num = SEM_ST;
            sops[0].sem_op = -1;
            sops[0].sem_flg = 0;
            if(semop(semid, sops, 1) == -1){
                ERROR_EXIT
            }
            
            /* aggiorno il taxi che ha raccolto più richieste di tutti */
            if(taxi.nreq > stat->n_high_req){
                stat->n_high_req = taxi.nreq;
                stat->pid_hreq_taxi = (long)getpid();
            }

            sops[0].sem_num = SEM_ST;
            sops[0].sem_op = 1;
            sops[0].sem_flg = 0;
            if(semop(semid, sops, 1) == -1){
                ERROR_EXIT
            }
            sigprocmask(SIG_UNBLOCK, &my_mask, NULL);


            alr_pos = taxi.where_taxi;
            alarm(param->so_timeout);
            start_time = time(NULL);

            while(taxi.where_taxi != queue.dest_cell && t){
                

                sigprocmask(SIG_BLOCK, &my_mask, NULL);
                sops[0].sem_num = SEM_MASTER;
                sops[0].sem_op = -1;
                sops[0].sem_flg = 0;

                if(semtimedop(semid, sops, 1, &tsop) == -1){
                    if(errno == EAGAIN){
                        if(t){
                            
                            sops[0].sem_num = SEM_START;
                            sops[0].sem_op = 1;
                            sops[0].sem_flg = 0;

                            sops[1].sem_num = SEM_TAXI;
                            sops[1].sem_op = 1;
                            sops[1].sem_flg = 0;
                            
                            if(semop(semid, sops, 2) == -1){
                                ERROR_EXIT
                            }
                            kill(getppid(), SIGUSR1);                            
                        }
                        city_map->m_cell[taxi.where_taxi].n_taxi_here -= 1;
                        
                        sops[0].sem_num = SEM_ST;
                        sops[0].sem_op = -1;
                        sops[0].sem_flg = 0;
                        if(semop(semid, sops, 1) == -1){
                            ERROR_EXIT
                        }
                        
                        stat->aborted_req += 1;

                        sops[0].sem_num = SEM_ST;
                        sops[0].sem_op = 1;
                        sops[0].sem_flg = 0;
                        if(semop(semid, sops, 1) == -1){
                            ERROR_EXIT
                        }
                        /*printf("taxi %ld "CRED"USCITA SEMTIMEDOP"CDEFAULT"\n",(long)getpid());*/
                    
                        exit(EXIT_FAILURE);
                        
                    }else{
                        ERROR_EXIT
                    }
                }else{
                
                    /* va alla cella destinazione. Fa un solo movimento la volta */

                        /*printf("Taxi %ld sono in %d, dest %d t vale = %d\n", (long)getpid(), taxi.where_taxi, queue.dest_cell, t);*/
                        go_cell(city_map, &taxi, queue.dest_cell);
                        if(taxi.where_taxi != alr_pos){
                            alarm(param->so_timeout);
                            alr_pos = taxi.where_taxi;
                        }

                    /*printf("taxi %ld in posizione %d\n", (long)getpid(), taxi.where_taxi);*/
                    
                    

                    sops[0].sem_num = SEM_MASTER;
                    sops[0].sem_op = 1;
                    sops[0].sem_flg = 0;
                    if(semop(semid, sops, 1) == -1){
                        ERROR_EXIT
                    }

                    treq.tv_sec = 0;
                    treq.tv_nsec = city_map->m_cell[taxi.where_taxi].cross_time;
                    if(nanosleep(&treq, &trem) == -1){
                        ERROR_EXIT
                    }                  
                    sigprocmask(SIG_UNBLOCK, &my_mask, NULL);
                    
                }
                
            
            }
            alarm(0);

            if(taxi.where_taxi == queue.dest_cell){
                end_time = time(NULL);
                taxi.time_request = end_time - start_time;
                sigprocmask(SIG_BLOCK, &my_mask, NULL);
                sops[0].sem_num = SEM_ST;
                sops[0].sem_op = -1;
                sops[0].sem_flg = 0;
                if(semop(semid, sops, 1) == -1){
                    ERROR_EXIT
                }
                /*printf(CGREEN"taxi %ld"CDEFAULT" ha completato il viaggio. taxiwh %d destcell %d\n", (long)getpid(), taxi.where_taxi, queue.dest_cell);*/
                stat->success_req += 1;
                /* aggiorno il taxi che ha impiegato più tempo per concludere una richiesta */
                if(taxi.time_request >= stat->high_time){  
                    stat->high_time = taxi.time_request;
                    stat->pid_htime_taxi = (long)getpid();
                }
        
                sops[0].sem_num = SEM_ST;
                sops[0].sem_op = 1;
                sops[0].sem_flg = 0;
                if(semop(semid, sops, 1) == -1){
                    ERROR_EXIT
                }
                sigprocmask(SIG_UNBLOCK, &my_mask, NULL);
            }else{
                sigprocmask(SIG_BLOCK, &my_mask, NULL);
                sops[0].sem_num = SEM_ST;
                sops[0].sem_op = -1;
                sops[0].sem_flg = 0;
                if(semop(semid, sops, 1) == -1){
                    ERROR_EXIT
                }
                
                stat->aborted_req += 1;
                
                sops[0].sem_num = SEM_ST;
                sops[0].sem_op = 1;
                sops[0].sem_flg = 0;
                if(semop(semid, sops, 1) == -1){
                    ERROR_EXIT
                }
                sigprocmask(SIG_UNBLOCK, &my_mask, NULL);
                /*printf(CRED"taxi %ld"CDEFAULT" non ha completato il viaggio. tempo scaduto t vale %d \n", (long)getpid(), t);*/
            }
        }
    }

    sigprocmask(SIG_BLOCK, &my_mask, NULL);
    sops[0].sem_num = SEM_ST;
    sops[0].sem_op = -1;
    sops[0].sem_flg = 0;
    if(semop(semid, sops, 1) == -1){
        ERROR_EXIT
    }
    
    /* aggiorno il taxi che ha passato più celle */
    if(taxi.traveled_cell > stat->high_ncells_crossed){
        stat->high_ncells_crossed = taxi.traveled_cell;
        stat->pid_hcells_taxi = (long)getpid();
    }
    
    sops[0].sem_num = SEM_ST;
    sops[0].sem_op = 1;
    sops[0].sem_flg = 0;
    if(semop(semid, sops, 1) == -1){
        ERROR_EXIT
    }
    sigprocmask(SIG_UNBLOCK, &my_mask, NULL);

    sops[0].sem_num = SEM_TAXI;
    sops[0].sem_op = 1;
    sops[0].sem_flg = 0;
    if(semop(semid, sops, 1) == -1){
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