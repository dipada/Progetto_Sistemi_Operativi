#include "../master/master.h"
#include "../mappa/mappa.h"

/* posiziona la cella SO_SOURCE  */
int place_source(map *city_map){
    int rand_position, i = 1;
    while(i > 0){
        /* sceglie una cella casualmente */
        if((rand_position = get_random(0,(SO_WIDTH*SO_HEIGHT-1))) == -1){
            fprintf(stderr, "Error: fail to generate random value for source\n");
            exit(EXIT_FAILURE);
        }

        if(!city_map->m_cell[rand_position].is_hole && !city_map->m_cell[rand_position].is_source){
            /* la cella non è un hole e non è un source */
            printf("source %ld in %d\n",(long)getpid(), rand_position);
            city_map->m_cell[rand_position].is_source = 1;
            city_map->m_cell[rand_position].pid_source = (long)getpid();
            i--;
        }
    }
    return rand_position;
}

/* genera una cella casuale che non sia hole e sia diversa dalla cella passata come argomento */
int get_dest_cell(map *city_map, int curr_source_pos){
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

/* genera una richiesta evitando la cella passatta. Inserisce la richiesta nella coda */
int make_request(map* city_map, int qid, int avoid_pos){
    struct request_queue queue;
    queue.start_cell = (long)getpid();
    queue.dest_cell = get_dest_cell(city_map, avoid_pos);
    return msgsnd(qid, &queue, sizeof(struct request_queue)- sizeof(long), IPC_NOWAIT);
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

