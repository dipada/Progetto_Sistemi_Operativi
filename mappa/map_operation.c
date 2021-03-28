#include "mappa.h"

/*carica la configurazione dal file selezionato 
struct configuration* load_configuration(int conf, struct configuration* param){
    printf("sono struct load\n");
}
*/

/* stampa la mappa */
void print_map(map * mappa){
    int i;
    for(i = 0; i < (SO_WIDTH*SO_HEIGHT); i++){
        printf("* ");
    }
}