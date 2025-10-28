#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h> 

#define NHIJOS 6

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
    if (oldhandler == SIG_ERR){
        error("signal:");
    }

    printf("Padre\n");

    pid_t root = getpid(), child[NHIJOS], pidHijo;

    for (int i = 0; i < NHIJOS; i++){
        child[i] = fork();
        if (!child[i]){
            pidHijo = i;
            break;
        }
        else if (child[i] == -1){
            error("Error en la creación del nuevo proceso");
        }
    }

    if(root==getpid()){
        usleep(1000);
        kill(child[NHIJOS-1], SIGUSR1);
        pause();
        printf("Padre\n");

        for (int j = 0; j < 2; j++){
            wait(NULL); // Espera a que los procesos hijos terminen
        }
    }else{
        pause();
        printf("Hijo %d PIDHIJO[%d]\n", pidHijo + 1, getpid());
        if(pidHijo == 0){
            kill(root, SIGUSR1);
        }else{
            kill(child[pidHijo-1], SIGUSR1);
        }

    }


    if (signal(SIGUSR1, oldhandler) == SIG_ERR){
        perror("signal: ");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}