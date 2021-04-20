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
    if(sig == SIGALRM){ 
        /* il taxi non si è mosso: - non è riuscito ad avere accesso al semaforo master
                                   - la cella successiva è piena */
        printf("taxi %ld "CYELLOW"alarm scattato"CDEFAULT"\n", (long)getpid());
        stat->aborted_req += 1;
        city_map->m_cell[taxi.where_taxi].n_taxi_here -= 1;
        sops[0].sem_num = SEM_TAXI;
        sops[0].sem_op = 1;
        sops[0].sem_flg = 0;
        
        sops[1].sem_num = SEM_MASTER;
        sops[1].sem_op = 1;
        sops[1].sem_flg = 0;
        if(semop(semid, sops, 2) == -1){
            ERROR_EXIT
        }

        exit(EXIT_FAILURE);
    }
}

int main(int argc, char** argv){

    int shm_map, shm_par, shm_stat, qid;

    struct parameters* param;

    struct request_queue queue;

    struct timespec treq, trem, tsop;

    struct sigaction sa;
    sigset_t my_mask;

    sa.sa_handler = &taxi_handler;
    sa.sa_flags = 0;
    sigfillset(&my_mask);
    sa.sa_mask = my_mask;

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
    if((semid = semget(SEMKEY, 3, 0)) == -1){
        ERROR_EXIT
    }

    /* coda di messaggi */
    if((qid = msgget(MSGKEY, 0)) == -1){
        ERROR_EXIT
    }
    
    sigaction(SIGTERM, &sa, NULL);
    /*printf("taxi %ld semmaster vale %d\n", (long)getpid(), semctl(semid, SEM_MASTER, GETVAL));*/

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
    if(semop(semid, sops, 1) == -1){
        ERROR_EXIT
    }

    sops[0].sem_num = SEM_TAXI;
    sops[0].sem_op = -1;
    sops[0].sem_flg = 0;
    if(semop(semid, sops, 1) == -1){
        ERROR_EXIT
    }
    printf("taxi %ld posizione %d\n", (long)getpid(), taxi.where_taxi);

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
        

        if(msgrcv(qid, &queue, (sizeof(struct request_queue) - sizeof(long)), taxi.pid_cell_taxi, IPC_NOWAIT) == -1){
            if(errno != ENOMSG){
                ERROR_EXIT
            }
        }else{
            
            sigprocmask(SIG_BLOCK, &my_mask, NULL);
            
            sops[0].sem_num = SEM_MASTER;
            sops[0].sem_op = -1;
            sops[0].sem_flg = 0;
            
            if(semop(semid, sops, 1) == -1){
                if(errno != EAGAIN){
                    ERROR_EXIT
                }else{
                    city_map->m_cell[taxi.where_taxi].n_taxi_here -= 1;

                    sops[0].sem_num = SEM_START;
                    sops[0].sem_op = 1;
                    sops[0].sem_flg = 0;

                    sops[1].sem_num = SEM_TAXI;
                    sops[1].sem_op = 1;
                    sops[1].sem_flg = 0;

                    /*sops[2].sem_num = SEM_MASTER;
                    sops[2].sem_op = 1;
                    sops[2].sem_flg = 0;*/

                    if(semop(semid, sops, 2) == -1){
                        ERROR_EXIT
                    }
                    kill(getppid(), SIGUSR1);
                    printf(CRED"taxi %ld"CDEFAULT" scattata semtimedop e semstart vale %d semmaster %d semtaxi %d\n", (long)getpid(), semctl(semid, SEM_START, GETVAL), semctl(semid, SEM_MASTER, GETVAL),semctl(semid, SEM_TAXI, GETVAL));
                    exit(EXIT_FAILURE);
                }
            }
            treq.tv_sec = 0;
            treq.tv_nsec = city_map->m_cell[taxi.where_taxi].cross_time;
            if(nanosleep(&treq, &trem) == -1){
                ERROR_EXIT
            }

            /*printf("taxi %ld in posizione %d\n", (long)getpid(), taxi.where_taxi);*/
            
            printf("Taxi %ld sono in %d, dest %d\n", (long)getpid(), taxi.where_taxi, queue.dest_cell);
            sops[0].sem_num = SEM_MASTER;
            sops[0].sem_op = 1;
            sops[0].sem_flg = 0;
            if(semop(semid, sops, 1) == -1){
                ERROR_EXIT
            }
            sigprocmask(SIG_UNBLOCK, &my_mask, NULL);
        }
    }

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