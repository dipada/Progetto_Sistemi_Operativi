#ifndef __MAPPA_H__
#define __MAPPA_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>


#define ERROR_EXIT     if(errno){fprintf(stderr,                            \
                        "[%s]-Line:%d-PID[%ld] >> Errore %d (%s)\n",        \
                        __FILE__ ,                                          \
                        __LINE__,                                           \
                        (long)getpid(),                                     \
                        errno,                                              \
                        strerror(errno));                                   \
                        exit(EXIT_FAILURE);}

#define BUFSIZE 128

/* parametri definiti a tempo di compilazione */
#define SO_WIDTH 5
#define SO_HEIGHT 4

/* path file di configurazione*/
#define CONF_FILE ("../file_configurazione/conf.txt")

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
    long cross_time;    /* tempo di attraversamento della cella */
    int capacity;       /* capacità della cella */
    int is_hole;        /* la cella è un hole 1 vero, 0 falso */
    int is_source;      /* la cella è un source 1 vero, 0 falso */
}map_cell;

/* struttura della mappa */
typedef struct{
    map_cell m_cell[SO_WIDTH*SO_HEIGHT];
}map;


/* carica la configurazione dal file passato col pathname */
void load_configuration(struct parameters* param, char * filename);

/* Inizializza la mappa, posiziona le SO_HOLES */
int initialize_map(map *city_map, const struct parameters *param);

/* riempie tutte le celle della mappa */
void fill_map(map *city_map, const struct parameters *param);

/* posiziona le n celle sohole nella mappa */
int place_hole(map *city_map, int n_hole, const int n_cells);

/* genera un numero random in un range [a,b] con a < b */
int get_random(int a, int b);

/* controlla se la cella sinistra a quella data è un hole. Ritorna 1 vero, 0 altrimenti */
int sx_cell_hole(map *city_map, int start_cell);

/* controlla se la cella destra a quella data è un hole. Ritorna 1 vero, 0 altrimenti */
int dx_cell_hole(map *city_map, int start_cell);

/* controlla se la cella sopra a quella data è un hole. Ritorna 1 vero, 0 altrimenti */
int up_cell_hole(map *city_map, int start_cell);

/* controlla se la cella sotto a quella data è un hole. Ritorna 1 vero, 0 altrimenti */
int dw_cell_hole(const map *city_map, const int start_cell);

/* controlla se la cella sinistra superiore (diagonale) a quella data è un hole. Ritorna 1 vero, 0 altrimenti */
int up_sx_cell_hole(map *city_map, int start_cell);

/* controlla se la cella destra superiore (diagonale) a quella data è un hole. Ritorna 1 vero, 0 altrimenti */
int up_dx_cell_hole(map *city_map, int start_cell);

/* controlla se la cella sinistra inferiore (diagonale) data è un hole. Ritorna 1 vero, 0 altrimenti */
int dw_sx_cell_hole(map *city_map, int start_cell);

/* controlla se la cella destra inferiore (diagonale) a quella data è un hole. Ritorna 1 vero, 0 altrimenti */
int dw_dx_cell_hole(map *city_map, int start_cell);

#endif