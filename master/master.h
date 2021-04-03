#ifndef __MASTER_H__
#define __MASTER_H__
#include "../mappa/mappa.h"
#include "../taxi/taxi.h"
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

union semun{
    int val; 
    unsigned short* array;
};

enum semaphores{SEM_MASTER, SEM_SOURCE, SEM_TAXI};


/* stampa la mappa evidenzianziando hole, sources e top_cells */
void print_map(map *city_map);

/* stampa lo stato di occupazione delle varie celle */
void print_status_cells(map *city_map);

/* controlla lo status di un processo */
int check_status(int status);
#endif