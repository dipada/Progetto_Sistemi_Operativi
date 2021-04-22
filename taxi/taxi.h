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
    int nreq;           /* numero di richieste raccolte dal taxi */
}taxi_t;


/* posiziona il taxi casualmente sulla mappa */
void place_taxi(map *city_map, taxi_t *taxi);

/* preleva una richiesta dalla coda di messaggi */
int take_request(map* city_map, int qid, taxi_t* taxi, struct request_queue* queue);

/* ----- MOVIMENTO TAXI ----- */

/* sposta il taxi di una cella verso la cella di destinazione */
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


/* fa evitare al taxi la cella hole inferiore, ritorna la posizione */
int skip_bot_hole(map *city_map, taxi_t *taxi);

/* fa evitare al taxi la cella hole superiore, ritorna la posizione */
int skip_top_hole(map *city_map, taxi_t *taxi);

/* fa evitare al taxi la cella hole destra, ritorna la posizione */
int skip_dx_hole(map *city_map, taxi_t *taxi);

/* fa evitare al taxi la cella hole sinistra, ritorna la posizione */
int skip_sx_hole(map *city_map, taxi_t *taxi);

/* genera un numero random in un range [a,b] con a < b */
int get_random(int a, int b);

#endif