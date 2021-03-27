

/* definiti a runtime */
#define SO_TAXI parameters.so_taxi                  /* numero di taxi presenti */
#define SO_SOURCE parameters.so_source              /* numero punti di origine */
#define SO_HOLES parameters.so_holes                /* numero celle inaccessibili */
#define SO_TOP_CELLS parameters.so_top_cells        /* numero celle maggiormente attraversate */
#define SO_CAP_MIN parameters.so_cap_min            /* capacità minima cella */
#define SO_CAP_MAX parameters.so_cap_max            /* capacità massima cella */
#define SO_TIMENSEC_MIN parameters.so_timensec_min  /* (nanosecondi) tempo minimo attraversamento cella */
#define SO_TIMENSEC_MAX parameters.so_timensec_max  /* (nanosecondi) tempo massimo attraversamento cella */
#define SO_TIMEOUT parameters.so_timeout            /* tempo inattività dei taxi prima della morte */
#define SO_DURATION parameters.so_duration          /* durata della simulazione */




/* struttura dei valori di configurazione caricati */
struct configuration{
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

/*char *pj_strdup(const char *str);
char *strdup2(const char *str);
void update_logfile(char *s);*/

#endif