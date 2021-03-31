#include "mappa.h"

int main(int argc, char **argv){

struct parameters param;
map city_map;

printf("sono dentro mappa\n");
/* caricamento dei parametri di configurazione da file*/
load_configuration(&param, CONF_FILE);

/* creazione della mappa */


if(initialize_map(&city_map, &param) == -1 ){
    fprintf(stderr, "Error: failed to initialize map\n");
}


print_map(&city_map);


exit(EXIT_SUCCESS);
}