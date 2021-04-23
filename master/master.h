#ifndef __MASTER_H__
#define __MASTER_H__
#ifndef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE
#endif
#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
#endif
#include "../mappa/mappa.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <time.h>

#define ERROR_EXIT     if(errno){fprintf(stderr,                            \
                        "[%s]-Line:%d-PID[%ld] >> Errore %d (%s)\n",        \
                        __FILE__ ,                                          \
                        __LINE__,                                           \
                        (long)getpid(),                                     \
                        errno,                                              \
                        strerror(errno));                                   \
                        exit(EXIT_FAILURE);}

/* Colori per le stampe */
#define CDEFAULT "\033[0m"
#define CRED "\033[0;31m"
#define CGREEN "\033[0;32m"
#define CYELLOW "\033[0;33m"

/* key per SHM */
#define SHMKEY_MAP 23899128
#define SHMKEY_PAR 19876980
#define SHMKEY_STAT 38435758
#define SEMKEY 65465445
#define MSGKEY 45829168

union semun{
    int val; 
    unsigned short* array;
};

struct statistic{
    int n_request;              /* numero di richieste effettuate */
    int success_req;            /* viaggi eseguiti con successo */
    int outstanding_req;        /* viaggi inevasi */
    int aborted_req;            /* viaggi abortiti */
    long pid_hcells_taxi;       /* pid del taxi che ha percorso più celle di tutti */
    int high_ncells_crossed;    /* numero celle attraversate dal taxi che ha percorso più celle di tutti */
    long pid_htime_taxi;        /* pid del taxi che ha impiegato più tempo di tutti per servire una richiesta */
    long high_time;             /* tempo impiegato dal taxi peggiore */
    long pid_hreq_taxi;         /* pid taxi che ha raccolto più richieste */
    int n_high_req;             /* numero di richieste raccolte dal taxi con più celle */
};

/* struttura per la coda di messaggi */
struct request_queue{
    long start_cell; /* cella di partenza utilizzato come mtype */
    int dest_cell;   /* cella di arrivo */
};

enum semaphores{SEM_MASTER, SEM_SOURCE, SEM_TAXI, SEM_START, SEM_ST};

/* inizializza la struttura delle statistiche */
void init_stat(struct statistic *stat);

/* stampa in percentuale lo stato di occupazione delle varie celle */
void print_status_cells(map *city_map);

/* stampa la mappa evidenzianziando hole, sources e top_cells */
void print_map(map *city_map, int n_top_cells);

/* ordina per numero di transizioni il vettore passato */
void sort_vector_transition(int* vet, int length, map* city_map);

/* verifica se la cella è una top cell */
int is_top_cell(int pos, const int* vet, int length);

/* dealloca le risorse in seguito ad un errore */
void free_all();

/* handler di segnali master */
void master_handler(int sig);

#endif