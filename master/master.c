/*
È presente un processo “master” che raccoglie le statistiche delle varie richieste eseguite.
Ogni secondo viene stampato a terminale lo stato di occupazione delle varie celle.
Alla fine della simulazione vengono stampati:
• numero viaggi (eseguiti con successo, inevasi e abortiti)
• la mappa con evidenziate le SO_SOURCES sorgenti e le SO_TOP_CELLS celle più attraversate
• il processo taxi che
1. ha fatto più strada (numero di celle) di tutti
2. ha fatto il viaggio più lungo (come tempo) nel servire una richiesta
3. ha raccolto più richieste/clienti
*/

#include "master.h"

int main(int argc, char **argv){

    int so_holes = -1, so_top_cells = -1, so_sources = -1, so_cap_min = -1, so_cap_max = -1, so_taxi = -1, so_timesec_min = -1 , so_timesec_max = -1, so_timeout = -1, so_duration = -1;
    int conf = -1, i;
    char c; 
    
    int * parameters[] = {&SO_HOLES, &SO_TOP_CELLS, &SO_SOURCES, &SO_CAP_MIN, &SO_CAP_MAX, &SO_TAXI, &SO_TIMENSEC_MIN, &SO_TIMENSEC_MAX, &SO_TIMEOUT, &SO_DURATION};

    /*
    Il programma può essere avviato nelle seguenti modalità:
        MODALITA RAPIDA, passando a linea di comando il numero della configurazione che si vuole caricare
        MODALITA STANDARD, avviando il programma senza parametri
    */
    if(argc > 2){
        fprintf(stderr,"[%s] >> Il programma può essere avviato solamente nelle seguenti modalità:\n  - Modalità standard: lanciando il programma senza parametri\n  - Modalità rapida: lanciando il programma passando un solo numero della seguente lista.\n\t[1] configurazione large\n\t[2] configurazione dense\n\t[3] configurazione personalizzata\n", __FILE__);
        exit(EXIT_FAILURE);
    }else if(argc == 2){ 
        sscanf(argv[1], "%d", &conf);
        if(!(conf >= 1 && conf <= 3)){
            fprintf(stderr,"[%s] >> valore inserito non valido. I valori possibili sono:\n[1] configurazione large\n[2] configurazione dense\n[3] configurazione personalizzata\n", __FILE__);
            exit(EXIT_FAILURE);
        }else{
            printf("[%s] >> valore accettato.\n", __FILE__);
        }
    }else{
        printf("[%s] >> Quale file di configurazione si intende caricare?\n[1] configurazione large\n[2] configurazione dense\n[3] configurazione personalizzata\nInserisci un valore >> ", __FILE__);
        while(!(conf >= 1 && conf <= 3)){
            c = getchar();
            conf = atol(&c);
            if(!(conf >= 1 && conf <= 3)){
                fprintf(stderr,"[%s] >> valore inserito non valido.\n", __FILE__);
                exit(EXIT_FAILURE);
            }else{
                printf("[%s] >> valore accettato.\n", __FILE__);
            }
        }
    }


    /* 
        caricamento della configurazione
    */
       
    load_configuration(conf, parameters, 10);
    

    exit(EXIT_SUCCESS);
}