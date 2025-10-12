#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h>

void *oldhandler;
void sighandler(int sig) {
    printf("sig %d capturada\n", sig);
}

void error(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(){
    pid_t child[2], root = getpid();
    int i;
    oldhandler = signal(SIGUSR1, sighandler);

    if (oldhandler == SIG_ERR) {
        error("signal:");
    }


    for(i=0; i<2; i++){
        child[i] = fork();
        if(!child[i]){
            break;
        }else if(child[i]==-1){
            error("Error en la creación del nuevo proceso");
        }
    }


    switch (i)
    {
    case 2: //proceso padre
        usleep(1000);
        kill(child[1], SIGUSR1);
        printf("[%d] Señal enviada por padre \n", getpid());
        pause();
        printf("[%d] Señal recibida por padre \n", getpid());

           // Esperar un poco antes de que los hijos terminen
            printf("\nEsperando un momento para mostrar el árbol...\n");
            sleep(2);

            // Mostrar árbol mientras los hijos siguen vivos
            char cmd[500];
            sprintf(cmd, "pstree -lp %d", getpid());
            system(cmd);

        for (int j = 0; j < 2; j++){
            wait(NULL); // Espera a que los procesos hijos terminen   
        }
        break;
    default: //logica para procesos hijos
        pause();
        printf("[%d] Terminando hijo\n", getpid());
        if (i==1){
            kill(child[0], SIGUSR1);
        }else{
            kill(root, SIGUSR1);
        }
        
        break;
    }


    if(signal(SIGUSR1, oldhandler)==SIG_ERR){
        perror("signal: ");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}
