#ifndef __MAPPA_H__
#define __MAPPA_H__
#include <stdio.h>

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


/*carica la configurazione dal file selezionato */
struct configuration* load_configuration(int conf, struct configuration* param);
#endif
