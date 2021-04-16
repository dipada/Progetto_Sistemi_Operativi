#include "../master/master.h"
#include "mappa.h"


int main(int argc, char **argv){

/****************************************************************************************************
*                                                                                                   *
*   Legge i parametri da file, crea, inizializza la mappa, posiziona le SO_HOLES e termina          *
*                                                                                                   *
****************************************************************************************************/

    struct parameters *param;
    map *city_map;
    
    struct sigaction sa;
    sigset_t my_mask;
    int shm_map, shm_par;
    
    sa.sa_handler = &map_handler;
    sa.sa_flags = 0;
    sigfillset(&my_mask);
    sa.sa_mask = my_mask;
    
    sigaction(SIGALRM, &sa, NULL);
    
    /* ID SHM */
    if((shm_map = shmget(SHMKEY_MAP, sizeof(map), 0)) == -1){
        ERROR_EXIT
    }
    if((shm_par = shmget(SHMKEY_PAR, sizeof(struct parameters), 0)) == -1){
        ERROR_EXIT
    }
    /* attach della shm */
    if((city_map = (map *) shmat(shm_map, NULL, 0)) == (void *) -1){
        ERROR_EXIT
    }
    if((param = (struct parameters *) shmat(shm_par, NULL, 0)) == (void *) -1){
        ERROR_EXIT
    }
    
    /* caricamento dei parametri di configurazione */
    load_configuration(param, CONF_FILE);
    
    /* inizializzo tutte le celle della mappa*/
    init_map(city_map, param);
    
    place_hole(city_map, param->so_holes, SO_WIDTH*SO_HEIGHT);
    
    /* detach della SHM */
    if(shmdt(city_map) == -1){
        ERROR_EXIT
    }
    if(shmdt(param) == -1){
        ERROR_EXIT
    }
    
    exit(EXIT_SUCCESS);
}

