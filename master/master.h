#ifndef __MASTER_H__
#define __MASTER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define ERROR_EXIT     if(errno){fprintf(stderr,                            \
                        "[%s]-Line:%d-PID[%ld] >> Errore %d (%s)\n",        \
                        __FILE__ ,                                          \
                        __LINE__,                                           \
                        (long)getpid(),                                     \
                        errno,                                              \
                        strerror(errno));                                   \
                        exit(EXIT_FAILURE);}

#define BUFSIZE 128

/* path file di configurazione e logfile di eventi*/
#define LOG_FILE ("../log/logfile.txt")
#define CONF_LARGE ("file_configurazioni/large.txt")
#define CONF_DENSE ("file_configurazioni/dense.txt")
#define CONF_CUSTOM ("file_configurazioni/custom.txt")
#define OPEN_CONF(x) ( (x == 1) ? CONF_LARGE : ((x == 2) ? CONF_DENSE : CONF_CUSTOM ) )

/* definiti a runtime */
#define SO_HOLES so_holes
#define SO_TOP_CELLS so_top_cells
#define SO_SOURCES so_sources
#define SO_CAP_MIN so_cap_min
#define SO_CAP_MAX so_cap_max
#define SO_TAXI so_taxi
#define SO_TIMENSEC_MIN so_timesec_min 
#define SO_TIMENSEC_MAX so_timesec_max
#define SO_TIMEOUT so_timeout
#define SO_DURATION so_duration
/* definiti a tempo di compilazione */
#define SO_WIDTH    1                /* larghezza della mappa */
#define SO_HEIGHT   1               /* altezza della mappa */

/* enum per indicizzare gli array */
enum flag {HOLES, TOP_CELLS, SOURCES, CAP_MIN, CAP_MAX, TAXI, TIMENSEC_MIN, TIMENSEC_MAX, TIMEOUT, DURATION, PASSED};

/* carica da file la configurazione */
void load_configuration(int conf, int ** arr_para, int len);

#endif