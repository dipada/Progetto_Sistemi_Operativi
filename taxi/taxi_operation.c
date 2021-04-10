#include "taxi.h"


/* inizializza la struttura delle statistiche */
void initialize_stat(struct statistic *stat){
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

/* posiziona celle SO_SOURCE e associa il pid del processo */
int place_source(map *city_map, int n_source, int n_cells){
    int rand_position;
    while(n_source > 0){
        /* sceglie una cella casualmente */
        if((rand_position = get_random(0,(n_cells-1))) == -1){
            fprintf(stderr, "Error: fail to generate random value for source\n");
            exit(EXIT_FAILURE);
        }

        if(!city_map->m_cell[rand_position].is_hole && !city_map->m_cell[rand_position].is_source){
            /* la cella non è un hole e non è un source */
            city_map->m_cell[rand_position].is_source = 1;
            city_map->m_cell[rand_position].pid_source = (long)getpid();
            n_source--;
        }
    }
    return rand_position;
}


/* posiziona il TAXI casualmente sulla mappa */
void place_taxi(map *city_map, taxi_t *taxi){
    int rand_position, i = 1;
    
    while(i > 0){
        /* sceglie casualmente la cella */
        if((rand_position = get_random(0, (SO_WIDTH*SO_HEIGHT - 1))) == -1){
            fprintf(stderr, "Error: fail to generate random value for taxi\n");
            exit(EXIT_FAILURE);
        }
        
        if(!city_map->m_cell[rand_position].is_hole && city_map->m_cell[rand_position].n_taxi_here < city_map->m_cell[rand_position].capacity){
            /* la cella non è un hole e non è piena*/
            city_map->m_cell[rand_position].n_taxi_here += 1;
            taxi->pid_taxi = (long)getpid();
            taxi->where_taxi = rand_position;
            taxi->pid_cell_taxi = city_map->m_cell[rand_position].pid_source; /* -1 se la cella non è source */
            taxi->traveled_cell = 0;
            taxi->time_request = -1;
            i--;
        }
    }
}

/* genera una cella casuale che non sia hole e sia diversa dalla cella passata come argomento */
int get_aim_cell(map *city_map, int curr_source_pos){
    int rand_position;
    
    do{
    if((rand_position = get_random(0, (SO_WIDTH*SO_HEIGHT - 1))) == -1){
            fprintf(stderr, "Error: fail to generate random value for aim_request\n");
            exit(EXIT_FAILURE);
        }
    }while(city_map->m_cell[rand_position].is_hole || rand_position == curr_source_pos);
    
    /* la cella non è un hole e non è la cella corrente */
    return rand_position;    
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

void go_cell(map* city_map, taxi_t *taxi, int goal_pos){
    int curr_pos = taxi->where_taxi;
    while(curr_pos != goal_pos){
        
        sleep(1);
        if(goal_pos > curr_pos){

            if(goal_pos >= curr_pos + SO_WIDTH){

                /*if(city_map->m_cell[curr_pos+SO_WIDTH].is_hole){
                    skip_hole(city_map, taxi, curr_pos+SO_WIDTH);
                }*/
                /* va alla cella inferiore se la cella dista più di SO_WIDTH posizioni e non è un HOLE */
                curr_pos = mv_dw(city_map, taxi, curr_pos);

            }else{
                
                if((goal_pos%SO_WIDTH) < (curr_pos%SO_WIDTH)){ /* siamo nella riga superiore in celle con resto maggiore di quella di arrivo */

                    curr_pos = mv_sx(city_map, taxi, curr_pos);
                }else{
                    /* la cella è a sinistra delle cella di arrivo */
                    curr_pos = mv_dx(city_map, taxi, curr_pos);
                }
            }
        }
        if(goal_pos < curr_pos){
            if(goal_pos <= curr_pos - SO_WIDTH){

                /* va alla cella superiore se la cella dista più di SO_WIDTH posizioni */
                    curr_pos = mv_up(city_map, taxi, curr_pos);
            }else{
                if(goal_pos%SO_WIDTH > curr_pos%SO_WIDTH){
                    curr_pos = mv_dx(city_map, taxi, curr_pos);
                }else{
                    curr_pos = mv_sx(city_map, taxi, curr_pos);
                }
            }
        }
    }
    printf("Arrivato a destinazione. Il taxi %ld si trova qui %d\n", (long)getpid(), taxi->where_taxi);
}


/* evita la cella HOLE sotto */
int skip_bot_hole(map *city_map, taxi_t *taxi){
    int curr_pos = taxi->where_taxi;
    if((curr_pos%SO_WIDTH) == SO_WIDTH - 1){ /* celle laterali destra */
        curr_pos = mv_sx(city_map, taxi, curr_pos);
    }else{
        curr_pos = mv_dx(city_map, taxi, curr_pos);
    }
    return curr_pos;
}

/* evita la cella HOLE sopra */
int skip_top_hole(map *city_map, taxi_t *taxi){
    int curr_pos = taxi->where_taxi;
    if((curr_pos%SO_WIDTH) == SO_WIDTH - 1){ /* celle laterali destra */
        curr_pos = mv_sx(city_map, taxi, curr_pos);
    }else{
        curr_pos = mv_dx(city_map, taxi, curr_pos);
    }
    return curr_pos;
}

/* evita la cella HOLE destra */
int skip_dx_hole(map *city_map, taxi_t *taxi){
    int curr_pos = taxi->where_taxi;
    if((curr_pos%SO_WIDTH) == SO_WIDTH - 1){ /* celle laterali destra */
        curr_pos = mv_sx(city_map, taxi, curr_pos);
    }else{
        curr_pos = mv_dx(city_map, taxi, curr_pos);
    }
    return curr_pos;
}
/* evita la cella HOLE sinistra */
int skip_sx_hole(map *city_map, taxi_t *taxi){
    int curr_pos = taxi->where_taxi;
    if((curr_pos%SO_WIDTH) == SO_WIDTH - 1){ /* celle laterali destra */
        curr_pos = mv_sx(city_map, taxi, curr_pos);
    }else{
        curr_pos = mv_dx(city_map, taxi, curr_pos);
    }
    return curr_pos;
}



/* sposta il taxi nella cella alla sua destra se non è hole e c'è spazio1, ritorna la nuova posizione altrimenti la stessa */
int mv_dx(map* city_map, taxi_t *taxi, int curr_pos){
    int new_pos = curr_pos + 1;
    
    if((curr_pos%SO_WIDTH) != SO_WIDTH -1 && city_map->m_cell[new_pos].n_taxi_here < city_map->m_cell[new_pos].capacity && !city_map->m_cell[new_pos].is_hole){
        /* non è una cella estrema destra. Nella cella destra c'è spazio per il taxi e non è hole */
        city_map->m_cell[curr_pos].n_taxi_here -= 1;
        city_map->m_cell[new_pos].n_taxi_here += 1;
        city_map->m_cell[new_pos].transitions += 1;
        taxi->pid_cell_taxi = city_map->m_cell[new_pos].pid_source;
        taxi->where_taxi = new_pos;
    
    printf("Il taxi %ld è andato a dx da %d ora è in %d\n",(long)getpid(), curr_pos, new_pos);

    return new_pos;
    }
    return curr_pos;    
}

/* sposta il taxi nella cella alla sua sinistra, ritorna la nuova posizione altrimenti la stessa */
int mv_sx(map* city_map, taxi_t *taxi, int curr_pos){
    int new_pos = curr_pos - 1;
    
    if((curr_pos%SO_WIDTH) != 0 && city_map->m_cell[new_pos].n_taxi_here < city_map->m_cell[new_pos].capacity && !city_map->m_cell[new_pos].is_hole){
        /* non è una cella estrema sinistra. Nella cella sinistra c'è spazio per il taxi e non è hole */
        city_map->m_cell[curr_pos].n_taxi_here -= 1;
        city_map->m_cell[new_pos].n_taxi_here += 1;
        city_map->m_cell[new_pos].transitions += 1;
        taxi->pid_cell_taxi = city_map->m_cell[new_pos].pid_source;
        taxi->where_taxi = new_pos;
    
    printf("Il taxi %ld è andato a sx da %d ora è in %d\n",(long)getpid(), curr_pos, new_pos);
    return new_pos;
    }
    return curr_pos;    
}

/* sposta il taxi nella cella sotto, ritorna la nuova posizione altrimenti la stessa */
int mv_dw(map* city_map, taxi_t *taxi, int curr_pos){
    int new_pos = curr_pos + SO_WIDTH;
    
    if(curr_pos < (SO_WIDTH*SO_HEIGHT)-SO_WIDTH && city_map->m_cell[new_pos].n_taxi_here < city_map->m_cell[new_pos].capacity && !city_map->m_cell[new_pos].is_hole){
        /* non è una cella estrema inferiore.Nella cella inferiore c'è spazio per il taxi e non è hole */
        city_map->m_cell[curr_pos].n_taxi_here -= 1;
        city_map->m_cell[new_pos].n_taxi_here += 1;
        city_map->m_cell[new_pos].transitions += 1;
        taxi->pid_cell_taxi = city_map->m_cell[new_pos].pid_source;
        taxi->where_taxi = new_pos;
    
    printf("Il taxi %ld è andato giu da %d ora è in %d\n",(long)getpid(), curr_pos, new_pos);
    return new_pos;
    }
    return curr_pos;    
}

/* sposta il taxi nella cella sopra, ritorna la nuova posizione altrimenti la stessa */
int mv_up(map* city_map, taxi_t *taxi, int curr_pos){
    int new_pos = curr_pos - SO_WIDTH;
    
    if(curr_pos > SO_WIDTH - 1 && city_map->m_cell[new_pos].n_taxi_here < city_map->m_cell[new_pos].capacity && !city_map->m_cell[new_pos].is_hole){
        /* non è una cella estrema destra. Nella cella superiore c'è spazio per il taxi e non è hole */
        city_map->m_cell[curr_pos].n_taxi_here -= 1;
        city_map->m_cell[new_pos].n_taxi_here += 1;
        city_map->m_cell[new_pos].transitions += 1;
        taxi->pid_cell_taxi = city_map->m_cell[new_pos].pid_source;
        taxi->where_taxi = new_pos;
    

    printf("Il taxi %ld è andato a su da %d ora è in %d\n",(long)getpid(), curr_pos, new_pos);
    return new_pos;
    }
    return curr_pos;
}


/* inizializza il semaforo a 1
int initSemAvailable(int semid, int semNum){
    union semun arg;

    arg.val = 1;
    return semctl(semid, semNum, SETVAL, arg);
}*/

/* inizializza il semaforo a 0 
int initSemInUse(int semid, int semNum){
    union semun arg;

    arg.val = 0;
    return semctl(semid, semNum, SETVAL, arg);
}*/

/* decrementa il semaforo di 1 
int reserveSem(int semid, int semnum){
    struct sembuf sops;

    sops.sem_num = semnum;
    sops.sem_op = -1;
    sops.sem_flg = 0;

    return semop(semid, &sops, 1);
}*/

/* incrementa il semaforo di 1 
int reserveSem(int semid, int semnum){
    struct sembuf sops;

    sops.sem_num = semnum;
    sops.sem_op = 1;
    sops.sem_flg = 0;

    return semop(semid, &sops, 1);
}*/