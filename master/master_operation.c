#include "master.h"

/* inizializza la struttura delle statistiche */
void init_stat(struct statistic *stat){
    stat->n_request = 0;
    stat->success_req = 0;
    stat->outstanding_req = 0;
    stat->aborted_req = 0;
    stat->pid_hcells_taxi = -1;
    stat->high_ncells_crossed = 0;
    stat->pid_htime_taxi = -1;
    stat->high_time = 0;
    stat->pid_hreq_taxi = -1;
    stat->n_high_req = 0;
}

/* stampa la mappa evidenzianziando hole, sources e top_cells */
void print_map(map *city_map){
    int register i,j; 
    
    printf("*-");
    /* stampo la riga iniziale della mappa */
    for(i=0; i<SO_WIDTH-1; i++){ 
	    printf("--");
	}
	printf("*\n");
    
    /* stampo le righe della mappa suddividendole per celle
       evidenzio SO_HOLES ( X rossa )
       SO_SOURCES ( S verde )
       SO_TOP_CELLS ( * YELLOW )  TODO
     */
    for(i = 0; i < SO_WIDTH*SO_HEIGHT; i++){
        if(city_map->m_cell[i].is_hole == 1 || city_map->m_cell[i].is_source == 1){
            if(city_map->m_cell[i].is_hole == 1){
                printf("|"CRED"X"CDEFAULT);
            }else{            
                printf("|"CGREEN"S"CDEFAULT);
            }
        }else{
            printf("| ");
        }
        if((i+1)%SO_WIDTH == 0){ /* se è l'ultima colonna della riga stampa "|" e stampa una riga di separazione */
            printf("|\n");
            printf("*-");
            for(j = 0; j < SO_WIDTH-1; j++){
                printf("--");
            }
            printf("*\n");
        }
    }
    printf("\n");
    printf("Legend: "CRED"X"CDEFAULT" Holes "CGREEN"S"CDEFAULT" Sources "CYELLOW"*"CDEFAULT" Top_Cells\n");
}

/* stampa lo stato di occupazione delle varie celle */
void print_status_cells(map *city_map){
    int register i, j;
    
    printf("cells capacity status\n");
    /* stampo la riga iniziale della mappa */
    
    printf(CRED"*"CDEFAULT"--");
    for(i=0; i<SO_WIDTH-1; i++){ 
	    printf("---");
	}
	printf(CRED"*"CDEFAULT"\n");
    
    /* stampo le righe della mappa suddividendole per celle */
    
    for(i = 0; i < SO_WIDTH*SO_HEIGHT; i++){
        
        if( (((float)city_map->m_cell[i].n_taxi_here/city_map->m_cell[i].capacity)*100) == 100){
            printf("  F");
        }else{
            printf(" %2.f", ((float)city_map->m_cell[i].n_taxi_here/city_map->m_cell[i].capacity)*100);
        }
        
        if((i+1)%SO_WIDTH == 0){ /* se è l'ultima colonna della riga stampa "|" e stampa una riga di separazione */
            printf("\n");
            printf(CRED"*"CDEFAULT"--");
            for(j = 0; j < SO_WIDTH-1; j++){
                printf("---");
            }
            printf(CRED"*"CDEFAULT"\n");
        }
    }
    

}

int check_status(int status){
    if(WIFEXITED(status)){
        return WEXITSTATUS(status);    
    }
    return -1;
}

