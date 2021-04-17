#ifndef __TAXI_H__
#define __TAXI_H__
#include "../mappa/mappa.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>


/* struttura del taxi */
typedef struct{
    long pid_taxi;      /* identificativo del taxi */
    int where_taxi;     /* la cella in cui si trova il taxi */
    long pid_cell_taxi; /* il pid della cella in cui si trova il taxi */
    long time_request;  /* il tempo più lungo di tutti impiegato dal taxi per servire una richiesta*/
    int traveled_cell;  /* numero di celle attraversate (somma di tutti i viaggi) */
}taxi_t;


/* posiziona il TAXI casualmente sulla mappa */
void place_taxi(map *city_map, taxi_t *taxi);

/* ----- MOVIMENTO TAXI ----- */

void go_cell(map* city_map, taxi_t *taxi, int goal_pos);

/* in base alla posizione del taxi cerca una cella source e ritorna la posizione */
int search_source(map *city_map, int cur_pos);

/* sposta il taxi nella cella alla sua destra, ritorna la nuova posizione altrimenti la stessa */
int mv_dx(map* city_map, taxi_t *taxi, int curr_pos);

/* sposta il taxi nella cella alla sua sinistra, ritorna la nuova posizione altrimenti la stessa */
int mv_sx(map* city_map, taxi_t *taxi, int curr_pos);

/* sposta il taxi nella cella sotto, ritorna la nuova posizione altrimenti la stessa */
int mv_dw(map* city_map, taxi_t *taxi, int curr_pos);

/* sposta il taxi nella cella sopra, ritorna la nuova posizione altrimenti la stessa */
int mv_up(map* city_map, taxi_t *taxi, int curr_pos);



int skip_bot_hole(map *city_map, taxi_t *taxi);
int skip_top_hole(map *city_map, taxi_t *taxi);
int skip_dx_hole(map *city_map, taxi_t *taxi);
int skip_sx_hole(map *city_map, taxi_t *taxi);





/* genera un numero random in un range [a,b] con a < b */
int get_random(int a, int b);

/* inizializza il semaforo a 1 */
int initSemAvailable(int semId, int semNum);

/* inizializza il semaforo a 0*/
int initSemInUse(int semId, int semNum);

/* decrementa il semaforo di 1 */
int reserveSem(int semId, int semNum);

/* incrementa il semaforo di 1 */
int releaseSem(int semId, int semNum);

#endif