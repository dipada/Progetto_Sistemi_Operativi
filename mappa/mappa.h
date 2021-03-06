#ifndef __MAPPA_H__
#define __MAPPA_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#define BUFSIZE 128

/* parametri definiti a tempo di compilazione */
#define SO_WIDTH 20
#define SO_HEIGHT 10

/* path file di configurazione*/
#define CONF_FILE ("file_configurazione/conf.txt")

/* struttura parametri di configurazione */
struct parameters{
    int so_taxi;
    int so_source;
    int so_holes;
    int so_top_cells;
    int so_cap_min;
    int so_cap_max;
    long so_timensec_min;
    long so_timensec_max;
    int so_timeout;
    int so_duration;
};

/* struttura cella della mappa */
typedef struct{
    long cross_time;        /* tempo di attraversamento della cella */
    int capacity;           /* capacità della cella */
    int is_hole;            /* la cella è un hole 1 vero, 0 falso */
    int is_source;          /* la cella è un source 1 vero, 0 falso */
    long pid_source;        /* sarà > 0 se associato un processo altrimenti -1 */
    int n_taxi_here;        /* numero di taxi presenti in questa cella */
    long transitions;       /* numero di volte che la cella è stata attraversata */
}map_cell;

/* struttura della mappa */
typedef struct{
    map_cell m_cell[SO_WIDTH*SO_HEIGHT];
}map;

/* handler gestione segnali mappa */
void map_handler(int sig);

/* carica la configurazione dal file passato col pathname */
void load_configuration(struct parameters* param, char * filename);

/* inizializzo tutte le celle della mappa*/
void init_map(map *city_map, const struct parameters *param);

/* posiziona le n celle sohole nella mappa */
void place_hole(map *city_map, int n_hole, const int n_cells);

/* posiziona la cella source */
int place_source(map *city_map);

/* genera una cella casuale che non sia hole e sia diversa dalla cella passata come argomento */
int get_dest_cell(map *city_map, int curr_source_pos);

/* genera una richiesta da terminale */
int make_request(map* city_map,int qid, int cur_pos);

/* genera un numero random in un range [a,b] con a < b */
int get_random(int a, int b);

/* controlla se la cella sinistra a quella data è un hole. Ritorna 1 vero, 0 altrimenti */
int sx_cell_hole(const map *city_map, const int start_cell);

/* controlla se la cella destra a quella data è un hole. Ritorna 1 vero, 0 altrimenti */
int dx_cell_hole(const map *city_map, const int start_cell);

/* controlla se la cella sopra a quella data è un hole. Ritorna 1 vero, 0 altrimenti */
int up_cell_hole(const map *city_map, const int start_cell);

/* controlla se la cella sotto a quella data è un hole. Ritorna 1 vero, 0 altrimenti */
int dw_cell_hole(const map *city_map, const int start_cell);

/* controlla se la cella sinistra superiore (diagonale) a quella data è un hole. Ritorna 1 vero, 0 altrimenti */
int up_sx_cell_hole(const map *city_map,const int start_cell);

/* controlla se la cella destra superiore (diagonale) a quella data è un hole. Ritorna 1 vero, 0 altrimenti */
int up_dx_cell_hole(const map *city_map,const int start_cell);

/* controlla se la cella sinistra inferiore (diagonale) data è un hole. Ritorna 1 vero, 0 altrimenti */
int dw_sx_cell_hole(const map *city_map,const int start_cell);

/* controlla se la cella destra inferiore (diagonale) a quella data è un hole. Ritorna 1 vero, 0 altrimenti */
int dw_dx_cell_hole(const map *city_map,const int start_cell);

#endif
