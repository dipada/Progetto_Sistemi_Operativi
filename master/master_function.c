#include "master.h"

/* legge da file e carica la configurazione nell'array passato */
/*void load_configuration(int conf, int **arr_param, int len){
    FILE *fp;
    char buf[BUFSIZE];
    
    int flags[] = {0,0,0,0,0,0,0,0,0,0,0};

    if((fp = fopen(OPEN_CONF(conf), "r")) == NULL){
        ERROR_EXIT
    }
    
    while(fgets(buf, BUFSIZE, fp) != NULL){
        
        if(sscanf(buf,"SO_HOLES=%d", arr_param[HOLES]) > 0){
           */ /* registrare i flags */
    /*    }
        if(sscanf(buf,"SO_TOP_CELLS=%d", arr_param[TOP_CELLS]) > 0){
            
        }
        if(sscanf(buf,"SO_SOURCES=%d", arr_param[SOURCES]) > 0){
            
        }
        if(sscanf(buf,"SO_CAP_MIN=%d", arr_param[CAP_MIN]) > 0){
            
        }
        if(sscanf(buf,"SO_CAP_MAX=%d", arr_param[CAP_MAX]) > 0){
        */    /* registrare i flags */
        /*}
        if(sscanf(buf,"SO_TAXI=%d", arr_param[TAXI]) > 0){
            
        }
        if(sscanf(buf,"SO_TIMENSEC_MIN=%d", arr_param[TIMENSEC_MIN]) > 0){
            
        }
        if(sscanf(buf,"SO_TIMENSEC_MAX=%d", arr_param[TIMENSEC_MAX]) > 0){
            
        }
        if(sscanf(buf,"SO_TIMEOUT=%d", arr_param[TIMEOUT]) > 0){
            
        }
        if(sscanf(buf,"SO_DURATION=%d", arr_param[DURATION]) > 0){
            
        }        
    }

    fclose(fp);
}
*/
/*
void print_charged_conf(int * parameters[], int len, int conf){
    char conf_name[7];
    char c = 'n';
    if(conf == 1){
        strcpy(conf_name, "large");
    }
    if(conf == 2){
        strcpy(conf_name, "dense");
    }
    if(conf == 3){
        strcpy(conf_name, "custom");
    }
    */
        
    /*while(c != 'y' && c != 'Y'){*/
       /* printf("Hai caricato la seguente configurazione\n\n");    
        printf("_______________________________\n");
        printf("parametro\t\t\"%s\"\n", conf_name);
        printf("_______________________________\n");
        printf("SOURCE \t\t%d\n", *(parameters[SOURCES]));
        printf("TAXI \t\t%d\n", *(parameters[TAXI]));
        printf("HOLES \t\t%d\n", (*parameters[HOLES]));
        printf("CAP MIN \t\t%d\n", *(parameters[CAP_MIN]));
        printf("CAP MAX \t\t%d\n", *(parameters[CAP_MAX]));
        printf("TIMEN MIN \t\t%d\n", *(parameters[TIMENSEC_MIN]));
        printf("TIMEN MAX \t\t%d\n", *(parameters[TIMENSEC_MAX]));
        printf("TIMEOUT \t\t%d\n", *(parameters[TIMEOUT]));
        printf("DURANTION \t\t%d\n", *(parameters[DURATION]));
        printf("TOP CELLS \t\t%d\n", *(parameters[TOP_CELLS]));
        printf("Confermare?[y/n]\n>> ");
        c = getchar();*/
    /*}*/
/*}*/