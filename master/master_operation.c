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
void print_map(map *city_map, int n_top_cells){
    int register i,j; 
    int temp = 1;
    int *tcel;
    
    if((tcel = (int *) malloc(sizeof(int)*n_top_cells) )== NULL){
        ERROR_EXIT
    }

    /* inizializza il vettore delle celle */
    for(i = 0; i < n_top_cells; i++){
        tcel[i] = i;
    }

    for(i = 0; i < SO_WIDTH*SO_HEIGHT; i++){ /* giro tutte le celle della mappa */
        sort_vector_transition(tcel, n_top_cells, city_map);
        if(city_map->m_cell[i].transitions > city_map->m_cell[tcel[0]].transitions){ /* confronto la prima posizione del vettore tcel */
            temp = 1;
            for(j = 0; j < n_top_cells; j++){ /* controllo che tale posizione non sia già presente nel vettore */
                    if(tcel[j] == i){
                        temp = 0;                        
                    }
                }
                if(temp){ /* solo se non già presente nel vettore tale posizione viene aggiunta */
                    tcel[0] = i;
                }
        }
    }

    printf("*-");
    /* stampo la riga iniziale della mappa */
    for(i=0; i<SO_WIDTH-1; i++){ 
	    printf("--");
	}
	printf("*\n");
    
    /* stampo le righe della mappa suddividendole per celle
       evidenzio SO_HOLES ( X rossa )
       SO_SOURCES ( S verde )
       SO_TOP_CELLS ( * giallo ) 
       SO_SOURCE AND SO_TOP_CELLS ( S gialla )
     */
    for(i = 0; i < SO_WIDTH*SO_HEIGHT; i++){
        if(city_map->m_cell[i].is_hole == 1 || city_map->m_cell[i].is_source == 1 || is_top_cell(i, tcel, n_top_cells)){
            if(city_map->m_cell[i].is_hole){
                printf("|"CRED"X"CDEFAULT);
            }else{
                if(city_map->m_cell[i].is_source){      
                    if(city_map->m_cell[i].is_source && is_top_cell(i, tcel, n_top_cells)){
                        printf("|"CYELLOW"S"CDEFAULT);
                    }else{
                        printf("|"CGREEN"S"CDEFAULT);
                    }
                }else{
                    printf("|"CYELLOW"*"CDEFAULT);
                }
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
    printf("Legend: "CRED"X"CDEFAULT" Holes "CGREEN"S"CDEFAULT" Sources "CYELLOW"*"CDEFAULT" Top_Cell "CYELLOW"S"CDEFAULT" Source and Top_Cell\n");
    fflush(stdout);
    free(tcel);
}

/* stampa lo stato di occupazione delle varie celle */
void print_status_cells(map *city_map){
    int register i, j;
    
    printf("cells capacity status percentage. F = 100%%\n");
    
    /* stampo la riga iniziale della mappa */
    printf(CRED"*"CDEFAULT"--");
    for(i=0; i<SO_WIDTH-1; i++){ 
	    printf("---");
	}
	printf(CRED"*"CDEFAULT"\n");
    
    /* stampo le righe della mappa suddividendole per celle */
    for(i = 0; i < SO_WIDTH*SO_HEIGHT; i++){
        
        if((((float)city_map->m_cell[i].n_taxi_here/city_map->m_cell[i].capacity)*100) == 100){
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
    printf("\n");
    fflush(stdout);
}

/* ordina per numero di transizioni il vettore passato */
void sort_vector_transition(int* vet, int length, map* city_map){
    int register i,j;
    int temp;
    for(i = 0; i < length - 1; i++){
        for(j = i + 1; j < length; j++ ){
            if(city_map->m_cell[vet[i]].transitions > city_map->m_cell[vet[j]].transitions){
                temp = vet[i];
                vet[i] = vet[j];
                vet[j] = temp;
            }
        }
    }
}

/* verifica se la cella è una top cell */
int is_top_cell(int pos, const int* vet, int length){
    int register i;
    int f = 0;
    for(i = 0; i < length; i++){
        if(vet[i] == pos){
            f = 1;
        }
    }
    return f;
}

/* dealloca le risorse in seguito ad un errore sulla mappa */
void free_all(){
    int shm_map, shm_par, shm_stat, qid, semid;
    if((shm_map = shmget(SHMKEY_MAP, sizeof(map), 0)) == -1){
        ERROR_EXIT
    }
    if((shm_par = shmget(SHMKEY_PAR, sizeof(struct parameters), 0)) == -1){
        ERROR_EXIT
    }
    if((shm_stat = shmget(SHMKEY_STAT, sizeof(struct statistic), 0)) == -1){
        ERROR_EXIT
    }
    if((semid = semget(SEMKEY, 5, 0)) == -1){
        ERROR_EXIT
    }
    if((qid = msgget(MSGKEY, 0)) == -1){
        ERROR_EXIT
    }

    if(msgctl(qid, IPC_RMID, NULL)){
        ERROR_EXIT
    }
    if(shmctl(shm_map,IPC_RMID, NULL) == -1){
        ERROR_EXIT
    }
    if(shmctl(shm_par,IPC_RMID, NULL) == -1){
        ERROR_EXIT
    }
    if(shmctl(shm_stat,IPC_RMID, NULL) == -1){
        ERROR_EXIT
    }
    if(semctl(semid,0, IPC_RMID) == -1){
        ERROR_EXIT
    }
}