#include "mappa.h"

/* carica la configurazione dal file passato col pathname */
void load_configuration(struct parameters* param, char * filename){
    
    long temp;
    char buf[BUFSIZE];
    FILE * fp; 
    if((fp = fopen(filename,"r")) == NULL){
        ERROR_EXIT
    }

    /*  lettura dei parametri da file 
        - Non importa l'ordine delle righe del file.
        - Scrivere il parametro numerico lasciando il simbolo '=' attacato alla stringa che lo precede
    */
    while(fgets(buf, BUFSIZE, fp)){
        if(sscanf(buf,"SO_HOLES=%ld", &temp)){
            param->so_holes = (int)temp;
        }
        if(sscanf(buf,"SO_TOP_CELLS=%ld", &temp)){
            param->so_top_cells = (int)temp;
        }
        if(sscanf(buf,"SO_SOURCES=%ld", &temp)){
            param->so_source = (int)temp;
        }
        if(sscanf(buf,"SO_CAP_MIN=%ld", &temp)){
            param->so_cap_min = (int)temp;
        }
        if(sscanf(buf,"SO_CAP_MAX=%ld", &temp)){
            param->so_cap_max = (int)temp;
        }
        if(sscanf(buf,"SO_TAXI=%ld", &temp)){
            param->so_taxi = (int)temp;
        }
        if(sscanf(buf,"SO_TIMENSEC_MIN=%ld", &temp)){
            param->so_timensec_min = temp;
        }
        if(sscanf(buf,"SO_TIMENSEC_MAX=%ld", &temp)){
            param->so_timensec_max = temp;
        }
        if(sscanf(buf,"SO_TIMEOUT=%ld", &temp)){
            param->so_timeout = (int)temp;
        }
        if(sscanf(buf,"SO_DURATION=%ld", &temp)){
            param->so_duration = (int)temp;
        }
    }

    /* controlli sui parametri*/
    if(param->so_holes >= (SO_WIDTH*SO_HEIGHT)){
        fprintf(stderr,"Error: invalid number of HOLES (%d) too big. Map has %d cells.\n", param->so_holes, (SO_WIDTH*SO_HEIGHT));
        exit(EXIT_FAILURE);
    }
    if(param->so_source >= (SO_WIDTH*SO_HEIGHT)){
        fprintf(stderr,"Error: invalid number of SOURCES (%d) too big. Map has %d cells.\n", param->so_source, (SO_WIDTH*SO_HEIGHT));
        exit(EXIT_FAILURE);
    }
    if(param->so_source <= 0){
        fprintf(stderr,"Error: invalid number of SOURCES (%d), must be greater than 0\n", param->so_source);
        exit(EXIT_FAILURE);
    }
    if(param->so_taxi <= 0){
        fprintf(stderr,"Error: invalid number of TAXI (%d), must be greater than 0\n", param->so_taxi);
        exit(EXIT_FAILURE);
    }
    if(param->so_cap_min <= 0 || param->so_cap_max <= 0 || param->so_cap_min > param->so_cap_max){
        fprintf(stderr,"Error: CAP_MIN and CAP_MAX parameters must be greater than 0. CAP_MIN must be lower or equal to CAP_MAX\n");
        exit(EXIT_FAILURE);
    }
    if(param->so_timensec_min <= 0 || param->so_timensec_max <= 0 || param->so_timensec_min > param->so_timensec_max){
        fprintf(stderr,"Error: TIMENSEC_MIN and TIMENSEC_MAX parameters must be greater than 0. TIMENSEC_MIN must be lower or equal to TIMENSEC_MAX\n");
        exit(EXIT_FAILURE);        
    }
    

    


    fclose(fp);

}