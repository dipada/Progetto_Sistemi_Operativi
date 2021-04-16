/* Il processo SOURCE si associa ad una cella della mappa che non sia HOLE o SOURCE */
#include "../mappa/mappa.h"
#include "../master/master.h"




int main(int argc, char** argv){
    int shm_map, shm_par, shm_stat, semid, qid, source_pos;

    map* city_map;
    struct statistic* stat;
    struct parameters* param;

    struct sembuf sops[2];

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
    if(semid = semget(SEMKEY, 3, 0) == -1){
        ERROR_EXIT
    }

    /* ID coda di messaggi */
    if((qid = msgget(MSGKEY, 0)) == -1){
        ERROR_EXIT
    }

    /*sigaction(SIGINT, &sa, NULL);*/

    /* posizionamento source */
    source_pos = place_source(city_map);
    printf("source %d in posizione %d\n", getpid(), source_pos);

    /* in attesa del master che autorizza l'avvio della simulazione */
    sops[0].sem_num = SEM_SOURCE;
    sops[0].sem_op = -1;
    sops[0].sem_flg = 0;
    if(semop(semid, sops, 1) == -1){
        ERROR_EXIT
    }

    printf("TAXI %ld, master ha concesso\n");

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


