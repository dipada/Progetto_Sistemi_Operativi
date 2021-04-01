#define _POSIX_C_SOURCE
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>


void handler(int sig){
    printf("Ricevuto %d %s\n", sig, strsignal(sig));
    exit(EXIT_SUCCESS);
}

int main(){
    struct sigaction sa;
    sigset_t my_mask;
    
    sa.sa_handler = &handler;
    sa.sa_flags = 0;
    sigfillset(&my_mask);
    sa.sa_mask = my_mask;

    sigaction(SIGINT, &sa, NULL);


    for(;;);

}