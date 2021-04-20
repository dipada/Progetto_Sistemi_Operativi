#include "../master/master.h"
#include "taxi.h"

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


/* preleva una richiesta dalla coda di messaggi */
int take_request(map* city_map, int qid, taxi_t* taxi, struct request_queue* queue){
    return msgrcv(qid, queue, (sizeof(struct request_queue) - sizeof(long)), taxi->pid_cell_taxi, IPC_NOWAIT);
}

/* in base alla posizione del taxi cerca una cella source e ritorna la posizione */
int search_source(map *city_map, int cur_pos){
    int source_pos;
    
    if(cur_pos > (SO_WIDTH*SO_HEIGHT)/2 || cur_pos == SO_WIDTH*SO_HEIGHT - 1){
        /* cerca a sinistra */
        while(!city_map->m_cell[cur_pos].is_source && cur_pos > 0){
            cur_pos -= 1;
        }
        /* se non ha ancora trovato la cella cerca nell'altro verso */
        if(!city_map->m_cell[cur_pos].is_source){
            while(!city_map->m_cell[(SO_WIDTH*SO_HEIGHT)/2 + 1].is_source && cur_pos < SO_WIDTH*SO_HEIGHT-1){
                cur_pos += 1;
            }
        }
        source_pos = cur_pos;

    }else{
        /* cella <= della metà, cerca a destra */
        while(!city_map->m_cell[cur_pos].is_source && cur_pos < SO_WIDTH*SO_HEIGHT -1 ){
            cur_pos += 1;
        }
        if(!city_map->m_cell[cur_pos].is_source){
            while(!city_map->m_cell[(SO_WIDTH*SO_HEIGHT)/2 - 1].is_source && cur_pos > 0){
                cur_pos -= 1;
            }
        }
        source_pos = cur_pos;
    }
    return source_pos;
    
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

/* sposta il taxi di una posizione verso la cella indicata */
void go_cell(map* city_map, taxi_t *taxi, int goal_pos){
    
    int curr_pos = taxi->where_taxi;
    
    if(goal_pos > curr_pos){
        
        if(goal_pos >= curr_pos + SO_WIDTH){
            
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

    /*taxi->where_taxi = curr_pos;*/
    
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