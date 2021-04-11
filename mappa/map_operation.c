#include "mappa.h"
#include "../master/master.h"

/* handler gestione ALARM per creazione mappa */
void map_signal_handler(int sig){
    if(sig == SIGALRM){
        fprintf(stderr, "[map_handler]: timer for generate map expired.\n");
        exit(EXIT_FAILURE);
    }
}

/* carica la configurazione dal file passato col pathname */
void load_configuration(struct parameters* param, char * filename){
    
    long temp;
    char buf[BUFSIZE];
    FILE * fp; 
    if((fp = fopen(filename,"r")) == NULL){
        ERROR_EXIT
    }

    /*  lettura dei parametri da file 
        - Non importa l'ordine delle righe del file.
        - Scrivere il parametro numerico lasciando il simbolo '=' attacato alla stringa che lo precede
    */
    while(fgets(buf, BUFSIZE, fp)){
        if(sscanf(buf,"SO_HOLES=%ld", &temp)){
            param->so_holes = (int)temp;
        }
        if(sscanf(buf,"SO_TOP_CELLS=%ld", &temp)){
            param->so_top_cells = (int)temp;
        }
        if(sscanf(buf,"SO_SOURCES=%ld", &temp)){
            param->so_source = (int)temp;
        }
        if(sscanf(buf,"SO_CAP_MIN=%ld", &temp)){
            param->so_cap_min = (int)temp;
        }
        if(sscanf(buf,"SO_CAP_MAX=%ld", &temp)){
            param->so_cap_max = (int)temp;
        }
        if(sscanf(buf,"SO_TAXI=%ld", &temp)){
            param->so_taxi = (int)temp;
        }
        if(sscanf(buf,"SO_TIMENSEC_MIN=%ld", &temp)){
            param->so_timensec_min = temp;
        }
        if(sscanf(buf,"SO_TIMENSEC_MAX=%ld", &temp)){
            param->so_timensec_max = temp;
        }
        if(sscanf(buf,"SO_TIMEOUT=%ld", &temp)){
            param->so_timeout = (int)temp;
        }
        if(sscanf(buf,"SO_DURATION=%ld", &temp)){
            param->so_duration = (int)temp;
        }
    }

    /* controlli sui parametri*/
    if(param->so_source > (SO_WIDTH*SO_HEIGHT)){
        fprintf(stderr,"Error: invalid number of SOURCES (%d) too big. Map has %d cells, available %d.\n", param->so_source, (SO_WIDTH*SO_HEIGHT), (SO_WIDTH*SO_HEIGHT)-param->so_holes);
        exit(EXIT_FAILURE);
    }
    if(param->so_source <= 0){
        fprintf(stderr,"Error: invalid number of SOURCES (%d), must be greater than 0\n", param->so_source);
        exit(EXIT_FAILURE);
    }
    if(param->so_taxi <= 0){
        fprintf(stderr,"Error: invalid number of TAXI (%d), must be greater than 0\n", param->so_taxi);
        exit(EXIT_FAILURE);
    }
    if(param->so_cap_min <= 0 || param->so_cap_max <= 0 || param->so_cap_min > param->so_cap_max){
        fprintf(stderr,"Error: CAP_MIN and CAP_MAX parameters must be greater than 0. CAP_MIN must be lower or equal to CAP_MAX\n");
        exit(EXIT_FAILURE);
    }
    if(param->so_timensec_min <= 0 || param->so_timensec_max <= 0 || param->so_timensec_min > param->so_timensec_max){
        fprintf(stderr,"Error: TIMENSEC_MIN and TIMENSEC_MAX parameters must be greater than 0. TIMENSEC_MIN must be lower or equal to TIMENSEC_MAX\n");
        exit(EXIT_FAILURE);        
    }

    if(param->so_timensec_min > 999999999 || param->so_timensec_max > 999999999){ 
        fprintf(stderr,"Error: TIMENSEC_MIN and TIMENSEC_MAX parameters must be lower or equal to 999999999 nsec\n");
        exit(EXIT_FAILURE);        
    }

    if((param->so_holes > ((SO_WIDTH*SO_HEIGHT)*15)/100 && param->so_holes > 1 ) || param->so_holes < 0){
        if(SO_WIDTH*SO_HEIGHT == 4){
            fprintf(stderr,"Error: numbers of HOLES can't be greater of 1 and lower than 0.\n");    
        }else{
            fprintf(stderr,"Error: numbers of HOLES can't be greater of %d and lower than 0.\n", ((SO_WIDTH*SO_HEIGHT)*15)/100);
        }
        exit(EXIT_FAILURE);
    }
    if((param->so_holes + param->so_source) > SO_WIDTH*SO_HEIGHT){
        fprintf(stderr,"Error: SO_HOLES and SO_SOURCE execeed numbers of cells in the map, actually map has [%d] cells\n", SO_WIDTH*SO_HEIGHT);
        exit(EXIT_FAILURE);
    }
    if(param->so_taxi > param->so_cap_min*(SO_WIDTH*SO_HEIGHT-param->so_holes)){
        fprintf(stderr,"Error: SO_TAXI execeed number of avaible position. max %d\n", param->so_cap_min*(SO_WIDTH*SO_HEIGHT-param->so_holes));
        exit(EXIT_FAILURE);
    }
    if(param->so_top_cells > (SO_WIDTH*SO_HEIGHT-param->so_holes)){
        fprintf(stderr,"Error: SO_TOP_CELL execeed number of non-hole cells. max %d\n", SO_WIDTH*SO_HEIGHT-param->so_holes);
        exit(EXIT_FAILURE);
    }
    if(param->so_timeout <= 0 || param->so_timeout > param->so_duration ){
        fprintf(stderr,"Error: SO_TIMEOUT must be almost 1 sec and lower than SO_DURATION\n");
        exit(EXIT_FAILURE);
    }


    fclose(fp);
}

/* inizializza tutte le celle della mappa */
void fill_map(map *city_map, const struct parameters *param){
    register int i;
    for(i = 0; i < SO_WIDTH*SO_HEIGHT; i++){
        city_map->m_cell[i].is_hole = 0;
        city_map->m_cell[i].is_source = 0;
        city_map->m_cell[i].pid_source = -1;
        if((city_map->m_cell[i].cross_time = get_random(param->so_timensec_min,param->so_timensec_max)) == -1){
            fprintf(stderr,"Error fill_map_function File:%s Line:%d: fail to initialize cell cross_time with random number", __FILE__, __LINE__);
            exit(EXIT_FAILURE);
        }
        if((city_map->m_cell[i].capacity = get_random(param->so_cap_min,param->so_cap_max)) == -1){
            fprintf(stderr,"Error fill_map_function File:%s Line:%d: fail to initialize cell capacity with random number", __FILE__, __LINE__);
            exit(EXIT_FAILURE);
        }
        city_map->m_cell[i].n_taxi_here = 0;  
        city_map->m_cell[i].transitions = 0;
    }
}

/* posiziona le n_hole nella mappa. n_cells è il numero totale complessivo di celle */
void place_hole(map *city_map, int n_hole, int n_cells){
    int rand_position;
    alarm(1);
    while(n_hole > 0){
        
        /* sceglie una cella casualmente */
        if((rand_position = get_random(0,(n_cells-1))) == -1){
            fprintf(stderr, "Error: fail to generate random value\n");
            exit(EXIT_FAILURE);
        }

        /* cella angolo alto sx */
        if(rand_position == 0){
            if((city_map->m_cell[rand_position].is_hole != 1) && !(dx_cell_hole(city_map,rand_position)) && !(dw_cell_hole(city_map,rand_position)) && !(dw_dx_cell_hole(city_map,rand_position))){ 
            /* la cella non è hole e quelle attorno nemmeno */
                n_hole--;
                city_map->m_cell[rand_position].is_hole = 1;
            }
        }

        /* cella angolo alto dx */
        if(rand_position == SO_WIDTH-1){
            if((city_map->m_cell[rand_position].is_hole != 1) && !(sx_cell_hole(city_map,rand_position)) && !(dw_cell_hole(city_map,rand_position)) && !(dw_sx_cell_hole(city_map,rand_position))){ 
            /* la cella non è hole e quelle attorno nemmeno */
                n_hole--;
                city_map->m_cell[rand_position].is_hole = 1;
            }
        }

        /* cella angolo basso sx */
          if(rand_position == (SO_WIDTH*SO_HEIGHT)-SO_WIDTH){
            if((city_map->m_cell[rand_position].is_hole != 1) && !(dx_cell_hole(city_map,rand_position)) && !(up_cell_hole(city_map,rand_position)) && !(up_dx_cell_hole(city_map,rand_position))){ 
            /* la cella non è hole e quelle attorno nemmeno */                
                n_hole--;
                city_map->m_cell[rand_position].is_hole = 1;
            }
        }

        /* cella angolo basso dx */
        if(rand_position == (SO_WIDTH*SO_HEIGHT)-1){
            if((city_map->m_cell[rand_position].is_hole != 1) && !(sx_cell_hole(city_map,rand_position)) && !(up_cell_hole(city_map,rand_position)) && !(up_sx_cell_hole(city_map,rand_position))){ 
            /* la cella non è hole e quelle attorno nemmeno */                
                n_hole--;
                city_map->m_cell[rand_position].is_hole = 1;
            }
        }

        /* celle laterali sx, sono esclusi gli angoli*/
        if(((rand_position%SO_WIDTH) == 0) && (rand_position != 0) && (rand_position != (SO_WIDTH*SO_HEIGHT)-SO_WIDTH)){
            if((city_map->m_cell[rand_position].is_hole != 1) && !(dx_cell_hole(city_map,rand_position)) && !(up_cell_hole(city_map,rand_position)) && !(dw_cell_hole(city_map,rand_position)) && !(dw_dx_cell_hole(city_map,rand_position)) && !(up_dx_cell_hole(city_map,rand_position))){ 
            /* la cella non è hole e quelle a destra, sopra, sotto e diagonali nemmeno */
                n_hole--;
                city_map->m_cell[rand_position].is_hole = 1;
            }
        }

        /* celle laterali dx */
        if(((rand_position%SO_WIDTH) == SO_WIDTH-1) && (rand_position != SO_WIDTH-1) && (rand_position != (SO_WIDTH*SO_HEIGHT)-1)){
            if((city_map->m_cell[rand_position].is_hole != 1) && !(sx_cell_hole(city_map,rand_position)) && !(up_cell_hole(city_map,rand_position)) && !(dw_cell_hole(city_map,rand_position)) && !(dw_sx_cell_hole(city_map,rand_position)) && !(up_sx_cell_hole(city_map,rand_position))){ 
            /* la cella non è hole e quelle a sinistra, sopra, sotto e diagonali nemmeno */
                n_hole--;
                city_map->m_cell[rand_position].is_hole = 1;
            }
        }

        /* celle top esclusi gli estremi (angoli) */
        if(((rand_position%SO_WIDTH) == rand_position) && (rand_position != 0) && (rand_position != (SO_WIDTH-1))){
            if((city_map->m_cell[rand_position].is_hole != 1) && !(sx_cell_hole(city_map,rand_position)) && !(dx_cell_hole(city_map,rand_position)) && !(dw_cell_hole(city_map,rand_position)) && !(dw_sx_cell_hole(city_map,rand_position)) && !(dw_dx_cell_hole(city_map,rand_position))){ 
            /* la cella non è hole e quelle a sinistra, sotto, destra e diagonali nemmeno */
                n_hole--;
                city_map->m_cell[rand_position].is_hole = 1;
            }
        }
        
        /* celle bottom */
        if((rand_position > (SO_WIDTH*SO_HEIGHT)-SO_WIDTH) && (rand_position < (SO_WIDTH*SO_HEIGHT)-1)){ 
           if((city_map->m_cell[rand_position].is_hole != 1) && !(sx_cell_hole(city_map,rand_position)) && !(dx_cell_hole(city_map,rand_position)) && !(up_cell_hole(city_map,rand_position)) && !(up_sx_cell_hole(city_map,rand_position)) && !(up_dx_cell_hole(city_map,rand_position))){
            /* la cella non è hole e quelle a sinistra, destra, sopra e diagonali nemmeno */
                n_hole--;
                city_map->m_cell[rand_position].is_hole = 1;
           }
        }

        /* celle centrali */
        if(((rand_position%SO_WIDTH)!= 0) && ((rand_position%SO_WIDTH)!= SO_WIDTH-1) && ((rand_position%SO_WIDTH) != rand_position) && ((rand_position < (SO_WIDTH*SO_HEIGHT)-SO_WIDTH))){
            /* sono escluse le celle superiori laterali ed inferiori */
            if((city_map->m_cell[rand_position].is_hole != 1) && !sx_cell_hole(city_map,rand_position) && !dx_cell_hole(city_map,rand_position)         \
                    && !up_cell_hole(city_map,rand_position) && !dw_cell_hole(city_map,rand_position) && !up_sx_cell_hole(city_map,rand_position)       \
                    && !up_dx_cell_hole(city_map,rand_position) && !dw_sx_cell_hole(city_map,rand_position) && !dw_dx_cell_hole(city_map,rand_position)){
                n_cells--;
                n_hole--;
                city_map->m_cell[rand_position].is_hole = 1;
            }
        }   

    }
    alarm(0); /* cancella l'alarm precedente */    
}

/* genera un numero random in un range [a,b] con a < b */
int get_random(int a, int b){
    static int flag_srand = 0; 
    /* il seme va impostato una sola volta poichè dipende dal tempo 
    quindi rand() restituisce lo stesso numero casuale */
    if(flag_srand == 0){
        flag_srand = 1;
        srand(time(NULL));
    }
    /* (b - a + 1) numeri nell'intervallo [a,b] */
    return (a <= b) ? (unsigned int)(rand()%(b - a + 1) + a) : -1;    
}

/* controlla se la cella sinistra a quella data è un hole. Ritorna 1 vero, 0 altrimenti */
int sx_cell_hole(const map *city_map, const int start_cell){
    return (city_map->m_cell[start_cell-1].is_hole == 1) ? 1 : 0;
}

/* controlla se la cella destra a quella data è un hole. Ritorna 1 vero, 0 altrimenti */
int dx_cell_hole(const map *city_map, const int start_cell){
    return (city_map->m_cell[start_cell+1].is_hole == 1) ? 1 : 0;
}

/* controlla se la cella sopra a quella data è un hole. Ritorna 1 vero, 0 altrimenti */
int up_cell_hole(const map *city_map, const int start_cell){
    return (city_map->m_cell[start_cell-SO_WIDTH].is_hole == 1) ? 1 : 0;
}

/* controlla se la cella sotto a quella data è un hole. Ritorna 1 vero, 0 altrimenti */
int dw_cell_hole(const map *city_map, const int start_cell){
    return (city_map->m_cell[start_cell+SO_WIDTH].is_hole == 1) ? 1 : 0;
}

/* controlla se la cella sinistra superiore (diagonale) a quella data è un hole. Ritorna 1 vero, 0 altrimenti */
int up_sx_cell_hole(const map *city_map, const int start_cell){
    return (city_map->m_cell[(start_cell-SO_WIDTH)-1].is_hole == 1) ? 1 : 0;
}

/* controlla se la cella destra superiore (diagonale) a quella data è un hole. Ritorna 1 vero, 0 altrimenti */
int up_dx_cell_hole(const map *city_map, const int start_cell){
    return (city_map->m_cell[(start_cell-SO_WIDTH)+1].is_hole == 1) ? 1 : 0;
}

/* controlla se la cella sinistra inferiore (diagonale) data è un hole. Ritorna 1 vero, 0 altrimenti */
int dw_sx_cell_hole(const map *city_map, const int start_cell){
    return (city_map->m_cell[(start_cell+SO_WIDTH)-1].is_hole == 1) ? 1 : 0;
}

/* controlla se la cella destra inferiore (diagonale) a quella data è un hole. Ritorna 1 vero, 0 altrimenti */
int dw_dx_cell_hole(const map *city_map, const int start_cell){
    return (city_map->m_cell[(start_cell+SO_WIDTH)+1].is_hole == 1) ? 1 : 0;
}


