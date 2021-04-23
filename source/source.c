/* Il processo SOURCE si associa ad una cella della mappa che non sia HOLE o SOURCE */
#include "../master/master.h"
#include "../mappa/mappa.h"

static int t = 1;

int semid, qid, source_pos;
map* city_map;

struct statistic* stat;

struct sembuf sops[2];

void source_handler(int sig);

int main(int argc, char** argv){
    
    int shm_map, shm_par, shm_stat;
    struct parameters* param;

    struct timespec treq, trem;
    
    struct sigaction sa;
    sigset_t my_mask;

    sa.sa_handler = &source_handler;
    sa.sa_flags = 0;
    sigfillset(&my_mask);
    sa.sa_mask = my_mask;

    /* recupero dell'ID e attach SHM */
    if((shm_map = shmget(SHMKEY_MAP, sizeof(map), 0)) == -1){
        ERROR_EXIT
    }
    if((shm_par = shmget(SHMKEY_PAR, sizeof(struct parameters), 0)) == -1){
        ERROR_EXIT
    }
    if((shm_stat = shmget(SHMKEY_STAT, sizeof(struct statistic), 0)) == -1){
        ERROR_EXIT
    }
    if((city_map = (map *)shmat(shm_map, NULL, 0)) == (void *) -1){
        ERROR_EXIT
    }
    if((param = (struct parameters*) shmat(shm_par, NULL, SHM_RDONLY)) == (void *) -1){
        ERROR_EXIT
    }
    if((stat = (struct statistic *) shmat(shm_stat, NULL, 0)) == (void *) -1){
        ERROR_EXIT
    }
    
    /* ID semafori */
    if((semid = semget(SEMKEY, 5, 0)) == -1){
        ERROR_EXIT
    }

    /* ID coda di messaggi */
    if((qid = msgget(MSGKEY, 0)) == -1){
        ERROR_EXIT
    }

    sigaction(SIGTERM, &sa, NULL);

    sops[0].sem_num = SEM_MASTER;
    sops[0].sem_op = -1;
    sops[0].sem_flg = 0;
    if(semop(semid, sops, 1) == -1){
        ERROR_EXIT
    }
    /* posizionamento source */
    source_pos = place_source(city_map);

    sops[0].sem_num = SEM_MASTER;
    sops[0].sem_op = 1;
    sops[0].sem_flg = 0;
    
    sops[1].sem_num = SEM_SOURCE;
    sops[1].sem_op = -1;
    sops[1].sem_flg = 0;
    if(semop(semid, sops, 2) == -1){
        ERROR_EXIT
    }

    /* in attesa del processo master che autorizza l'avvio della simulazione */
    /* il master è in attesa dello 0 di SEM_SOURCE */
    sops[0].sem_num = SEM_START;
    sops[0].sem_op = -1;
    sops[0].sem_flg = 0;
    if(semop(semid, sops, 1) == -1){
        ERROR_EXIT
    }

    sigaction(SIGINT, &sa, NULL);

    while(t){
    
        /* genera richieste taxi con un intervallo variabile tra 1 nsec - 1 sec */
        treq.tv_sec = 0;
        treq.tv_nsec = get_random(1, 999999999);
        sigprocmask(SIG_BLOCK, &my_mask, NULL);
        if(nanosleep(&treq, &trem) == -1){        
            ERROR_EXIT
        }
        sigprocmask(SIG_UNBLOCK, &my_mask, NULL);

        /* preparazione e invio della richiesta */    
        if(make_request(city_map, qid, source_pos) == -1){
            if(errno != EAGAIN){
                sigprocmask(SIG_BLOCK, &my_mask, NULL);
                sops[0].sem_num = SEM_ST;
                sops[0].sem_op = -1;
                sops[0].sem_flg = 0;
                if(semop(semid, sops, 1) == -1){
                    ERROR_EXIT
                }
                
                stat->outstanding_req += 1;

                sops[0].sem_num = SEM_ST;
                sops[0].sem_op = 1;
                sops[0].sem_flg = 0;
                if(semop(semid, sops, 1) == -1){
                    ERROR_EXIT
                }
                sigprocmask(SIG_UNBLOCK, &my_mask, NULL);
                exit(EXIT_FAILURE);
            }
        }else{
            sigprocmask(SIG_BLOCK, &my_mask, NULL);
            sops[0].sem_num = SEM_ST;
            sops[0].sem_op = -1;
            sops[0].sem_flg = 0;
            if(semop(semid, sops, 1) == -1){
                ERROR_EXIT
            }

            /* registra l'avvenuta richiesta */
            stat->n_request += 1;
            

            sops[0].sem_num = SEM_ST;
            sops[0].sem_op = 1;
            sops[0].sem_flg = 0;
            if(semop(semid, sops, 1) == -1){
                ERROR_EXIT
            }
            sigprocmask(SIG_UNBLOCK, &my_mask, NULL);
            
        }    
    }

    sops[0].sem_num = SEM_SOURCE;
    sops[0].sem_op = 1;
    sops[0].sem_flg = 0;
    if(semop(semid, sops, 1) == -1){
        ERROR_EXIT
    }

    /* detach delle SHM */
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


void source_handler(int sig){
    if(sig == SIGTERM){
        t = 0;
    }
    if(sig == SIGINT){
        if(make_request(city_map, qid, source_pos) == -1){
            if(errno == EAGAIN){
                fprintf(stdout, "\nsource [%d] >> "CRED"Can't register your request. Queue is full."CDEFAULT"\n", source_pos);
                fflush(stdout);
            }else{
                ERROR_EXIT
            }
        }else{
            sops[0].sem_num = SEM_ST;
            sops[0].sem_op = -1;
            sops[0].sem_flg = 0;
            if(semop(semid, sops, 1) == -1){
                ERROR_EXIT
            }
            /* registra l'avvenuta richiesta */
            stat->n_request += 1;
            fprintf(stdout, "\nsource [%d] >> "CGREEN"Request registered with success"CDEFAULT"\n", source_pos);
            fflush(stdout);
            sops[0].sem_num = SEM_ST;
            sops[0].sem_op = 1;
            sops[0].sem_flg = 0;
            if(semop(semid, sops, 1) == -1){
                ERROR_EXIT
            }
            
        }
    }
}