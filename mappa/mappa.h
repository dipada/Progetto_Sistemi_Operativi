#ifndef __MAPPA_H__
#define __MAPPA_H__
#include <stdio.h>
#include <stdlib.h>



#define ERROR_EXIT     if(errno){fprintf(stderr,                            \
                        "[%s]-Line:%d-PID[%ld] >> Errore %d (%s)\n",        \
                        __FILE__ ,                                          \
                        __LINE__,                                           \
                        (long)getpid(),                                     \
                        errno,                                              \
                        strerror(errno));                                   \
                        exit(EXIT_FAILURE);}

/* parametri definiti a tempo di compilazione */
#define SO_WIDTH 20
#define SO_HEIGHT 10

/* path file di configurazione*/
#define CONF_FILE ("..file_configurazione/conf.txt")

/* struttura parametri di configurazione */
struct{
    long so_taxi;
    long so_source;
    int so_holes;
    int so_top_cells;
    int so_cap_min;
    int so_cap_max;
    unsigned long so_timensec_min;
    unsigned long so_timensec_max;
    int so_timeout;
    int so_duration;
}parameters;

/* struttura cella della mappa */
typedef struct _cell{
    unsigned long cross_time;   /* tempo di attraversamento della cella */
    unsigned short capacity;    /* capacità della cella */
}map_cell;

/* struttura della mappa */
typedef struct{
    map_cell m_cell[SO_WIDTH*SO_HEIGHT];
}map;


/*carica la configurazione dal file selezionato 
struct configuration* load_configuration(int conf, struct configuration* param);
*/

/* stampa la mappa */
void print_map(map * mappa);

#endif