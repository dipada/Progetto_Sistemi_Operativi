#include "mappa.h"

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
    if(param->so_holes >= (SO_WIDTH*SO_HEIGHT)){
        fprintf(stderr,"Error: invalid number of HOLES (%d) too big. Map has %d cells.\n", param->so_holes, (SO_WIDTH*SO_HEIGHT));
        exit(EXIT_FAILURE);
    }
    if(param->so_source >= (SO_WIDTH*SO_HEIGHT)){
        fprintf(stderr,"Error: invalid number of SOURCES (%d) too big. Map has %d cells.\n", param->so_source, (SO_WIDTH*SO_HEIGHT));
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

    fclose(fp);
}


/* Inizializza la mappa, posiziona le SO_HOLES */
int initialize_map(map *city_map, const struct parameters *param){
    
    /*
    printf("SO HOLES %d\nSO_TOP_CELLS %d\nSO SOURCES %d\nSO_CAP_MIN %d\n",param->so_holes, param->so_top_cells, param->so_source, param->so_cap_min );
    printf("SO CAP_MAX %d\nSO_TAXI %d\nSO TIMENSEC_MIN %ld\nSO_TIMENSEC_MAX %ld\n",param->so_cap_max, param->so_taxi, param->so_timensec_min, param->so_timensec_max );
    printf("SO TIMEOUT %d\nSO_DURATION %d\n", param->so_timeout, param->so_duration);
    */

    fill_map(city_map,param);

    /*
    place_hole(city_map, param->so_holes, (SO_WIDTH*SO_HEIGHT));
    */
    return 1;
}

/* riempie tutte le celle della mappa */
void fill_map(map *city_map, const struct parameters *param){
    int register i;
    for(i = 0; i < SO_WIDTH*SO_HEIGHT; i++){
        city_map->m_cell[i].is_hole = 0;
        city_map->m_cell[i].is_source = 0;
        city_map->m_cell[i].cross_time = get_random(param->so_timensec_min,param->so_timensec_max);
        city_map->m_cell[i].capacity = get_random(param->so_cap_min,param->so_cap_max);
    }
}

/* posiziona le n celle sohole nella mappa */
int place_hole(map *city_map, int n_hole, const int n_cells){
    int i, rand_position;

    for(i = 0; i < n_hole; i++){
        if((rand_position = get_random(0,n_cells-1)) == -1){
            fprintf(stderr, "Error: fail to generate random value\n");
            exit(EXIT_FAILURE);
        }
        /* cella nell'angolo alto sinistro */
        if(rand_position == 0){
            if(dw_cell_hole(city_map, rand_position) == 1){
                printf("is_hole1 vale %d\n", city_map->m_cell[rand_position+SO_WIDTH].is_hole);
            }else{
                printf("is_hole2 vale %d\n", city_map->m_cell[rand_position+SO_WIDTH].is_hole);
            }
        }
    }
    return 1;
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
int sx_cell_hole(map *city_map, int start_cell){
    printf("cucu1");
    return (city_map->m_cell[start_cell-1].is_hole == 1) ? 1 : 0;
}

/* controlla se la cella destra a quella data è un hole. Ritorna 1 vero, 0 altrimenti */
int dx_cell_hole(map *city_map, int start_cell){
    printf("cucu2");
    return (city_map->m_cell[start_cell+1].is_hole == 1) ? 1 : 0;
}

/* controlla se la cella sopra a quella data è un hole. Ritorna 1 vero, 0 altrimenti */
int up_cell_hole(map *city_map, int start_cell){
    printf("cucu3");
    return (city_map->m_cell[start_cell-SO_WIDTH].is_hole == 1) ? 1 : 0;
}

/* controlla se la cella sotto a quella data è un hole. Ritorna 1 vero, 0 altrimenti */
int dw_cell_hole(const map *city_map, const int start_cell){
    printf("cucu4");
    return (city_map->m_cell[start_cell+SO_WIDTH].is_hole == 1) ? 1 : 0;
}

/* controlla se la cella sinistra superiore (diagonale) a quella data è un hole. Ritorna 1 vero, 0 altrimenti */
int up_sx_cell_hole(map *city_map, int start_cell){
    return (city_map->m_cell[(start_cell-SO_WIDTH)-1].is_hole == 1) ? 1 : 0;
}

/* controlla se la cella destra superiore (diagonale) a quella data è un hole. Ritorna 1 vero, 0 altrimenti */
int up_dx_cell_hole(map *city_map, int start_cell){
    return (city_map->m_cell[(start_cell-SO_WIDTH)+1].is_hole == 1) ? 1 : 0;
}

/* controlla se la cella sinistra inferiore (diagonale) data è un hole. Ritorna 1 vero, 0 altrimenti */
int dw_sx_cell_hole(map *city_map, int start_cell){
    return (city_map->m_cell[(start_cell+SO_WIDTH)-1].is_hole == 1) ? 1 : 0;
}

/* controlla se la cella destra inferiore (diagonale) a quella data è un hole. Ritorna 1 vero, 0 altrimenti */
int dw_dx_cell_hole(map *city_map, int start_cell){
    return (city_map->m_cell[(start_cell+SO_WIDTH)+1].is_hole == 1) ? 1 : 0;
}


