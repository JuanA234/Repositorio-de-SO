#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h>

void *oldhandler;
void sighandler(int sig)
{
    // printf("sig %d capturada\n", sig);
}

void error(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}


int main(){
    
    oldhandler = signal(SIGUSR1, sighandler);

    if (oldhandler == SIG_ERR)
    {
        error("signal:");
    }

    return EXIT_SUCCESS;
}