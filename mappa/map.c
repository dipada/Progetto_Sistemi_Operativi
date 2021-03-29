#include "mappa.h"

int main(int argc, char **argv){

/* carica la configurazione da file */
struct parameters param;
map city_map;
int i = 0;

load_configuration(&param, CONF_FILE);

/* creazione della mappa */


/*
printf("SO HOLES %d\nSO_TOP_CELLS %d\nSO SOURCES %d\nSO_CAP_MIN %d\n",param.so_holes, param.so_top_cells, param.so_source, param.so_cap_min );
printf("SO CAP_MAX %d\nSO_TAXI %d\nSO TIMENSEC_MIN %ld\nSO_TIMENSEC_MAX %ld\n",param.so_cap_max, param.so_taxi, param.so_timensec_min, param.so_timensec_max );
printf("SO TIMEOUT %d\nSO_DURATION %d\n", param.so_timeout, param.so_duration);
*/

/*
while (i < SO_WIDTH*SO_HEIGHT)
{
    city_map.m_cell[i++].cross_time = i;     
}

i = 0;
while (i < SO_WIDTH*SO_HEIGHT)
{
   printf("%ld ",city_map.m_cell[i++].cross_time);      
}

city_map.m_cell[4].cross_time = 48;
*/


if(initialize_map(&city_map, &param) == -1 ){
    fprintf(stderr, "Error: failed to initialize map\n");
}

for(i = 0; i < SO_HEIGHT*SO_WIDTH; i++){
        if(city_map.m_cell[i].is_hole == 1)
            printf("cella %d è so hole\n", i);
    }
/*initialize_map(&city_map, &param);*/

printf("\n\n\n");
for(i = 0; i < SO_HEIGHT*SO_WIDTH; i++){
    printf("Cella %d:\tis_hole %d\tis_source %d\tcross_time %ld\tcapacity %d\n",\
    i, city_map.m_cell[i].is_hole, city_map.m_cell[i].is_source,city_map.m_cell[i].cross_time, city_map.m_cell[i].capacity);   
}

exit(EXIT_SUCCESS);
}