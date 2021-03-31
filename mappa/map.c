#include "mappa.h"
#include "../master/master.h"
#include <signal.h>

int main(int argc, char **argv){

struct parameters param;
map *city_map;
int shmid;

if((shmid = shmget(SHMKEY, sizeof(map), 0)) == -1){
    ERROR_EXIT
}

if((city_map = (map *) shmat(shmid, NULL, 0)) == (void *) -1){
    ERROR_EXIT
}

/* caricamento dei parametri di configurazione da file nella struttura param */
load_configuration(&param, CONF_FILE);

/* creazione della mappa */
if(initialize_map(city_map, &param) == -1 ){
    fprintf(stderr, "Error: failed to initialize map\n");
}



exit(EXIT_SUCCESS);
}

