#ifndef __TAXI_H__
#define __TAXI_H__
#include "../mappa/mappa.h"
#include "../master/master.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

/* struttura per la richiesta taxi */
struct request_queue{
    long start_cell; /* cella di partenza utilizzato come mtype */
    long aim_cell;   /* cella di arrivo */
};

/* struttura del taxi */
typedef struct{
    long pid_taxi;      /* identificativo del taxi */
    int where_taxi;     /* la cella in cui si trova il taxi */
    long pid_cell_taxi; /* il pid della cella in cui si trova il taxi */
    long time_request;  /* il tempo più lungo di tutti impiegato dal taxi per servire una richiesta*/
    int traveled_cell;  /* numero di celle attraversate (somma di tutti i viaggi) */
}taxi_t;

struct statistic{
    int n_request;              /* numero di richieste effettuate */
    int success_req;            /* viaggi eseguiti con successo */
    int outstanding_req;        /* viaggi inevasi */
    int aborted_req;            /* viaggi abortiti */
    long pid_hcells_taxi;       /* pid del taxi che ha percorso più celle */
    int high_ncells_crossed;    /* numero celle attraversate dal taxi che ah percorso più celle taxi */
    long pid_htime_taxi;        /* pid del taxi che ha impiegato più tempo di tutti */
    long high_time;             /* tempo impiegato dal taxi peggiore */
    long pid_hreq_taxi;         /* pid taxi che ha raccolto più richieste */
    int n_high_req;             /* numero di richieste raccolte dal taxi con più celle */
};


/* inizializza la struttura delle statistiche */
void initialize_stat(struct statistic *stat);

/* posiziona celle SO_SOURCE e associa il pid del processo alla cella. Ritorna la poszione della cella SOURCE*/
int place_source(map *city_map, int n_source, int n_cells);

/* posiziona il TAXI casualmente sulla mappa */
void place_taxi(map *city_map, taxi_t *taxi);

/* genera una cella casuale che non sia hole e sia diversa dalla cella passata come argomento */
int get_aim_cell(map *city_map, int curr_source_pos);

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