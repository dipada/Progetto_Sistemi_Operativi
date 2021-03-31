#ifndef __MASTER_H__
#define __MASTER_H__
#include "../mappa/mappa.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>



#define SHMKEY 23899128

/* stampa la mappa evidenzianziando hole, sources e top_cells */
void print_map(map *city_map);

int check_status(int status);
#endif