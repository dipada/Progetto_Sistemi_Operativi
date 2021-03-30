#include "mappa.h"

int main(int argc, char **argv){

/* carica la configurazione da file */
struct parameters param;
map city_map;


/* caricamento dei parametri di configurazione*/
load_configuration(&param, CONF_FILE);

/* creazione della mappa */

for(;;){
if(initialize_map(&city_map, &param) == -1 ){
    fprintf(stderr, "Error: failed to initialize map\n");
}


print_map(&city_map);
sleep(1);
}

exit(EXIT_SUCCESS);
}