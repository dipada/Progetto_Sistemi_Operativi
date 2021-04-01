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
#include <sys/wait.h>

void handler(int sig){
    printf("\nPID %d Ricevuto %d %s\n", getpid(), sig, strsignal(sig));
    exit(EXIT_SUCCESS);
}

int main(){
    struct sigaction sa;
    sigset_t my_mask;
    
    sa.sa_handler = &handler;
    sa.sa_flags = 0;
    sigfillset(&my_mask);
    sa.sa_mask = my_mask;

    

    if(fork() == 0){
    sigaction(SIGINT, &sa, NULL);
        printf("PID %dn", getpid());
        fflush(stdout);
        for(;;);
    }
    
    printf("PARENT %d\n", getpid());
    wait(NULL);
    exit(EXIT_SUCCESS);

}