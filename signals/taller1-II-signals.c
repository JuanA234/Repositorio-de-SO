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

int main()
{

    int n;

    printf("Ingresar número de iteraciones: ");
    scanf("%d", &n);

    oldhandler = signal(SIGUSR1, sighandler);

    if (oldhandler == SIG_ERR)
    {
        error("signal:");
    }

    printf("Padre [%d] \n", getpid());

    pid_t child[2], root = getpid();
    for (int j = 0; j < n; j++){
        int i;
        if(root==getpid()){
            for (i = 0; i < 2; i++){
            child[i] = fork();
            if (!child[i])
            {
                break;
            }
            else if (child[i] == -1)
            {
                error("Error en la creación del nuevo proceso");
            }
        }

        }
        if(i==2){ // proceso padre
            usleep(1000);
            kill(child[1], SIGUSR1);
            pause();
            printf("Padre [%d] \n", getpid());

            for (int j = 0; j < 2; j++)
            {
                wait(NULL); // Espera a que los procesos hijos terminen
            }
        }
        else{// logica para procesos hijos
            pause();
            printf(" hijo%d [%d]\n", i + 1, getpid());
            if (i == 1)
            {
                pid_t pidhijo = fork();
                switch (pidhijo)
                {
                case -1:
                    error("Error en la creación del nuevo proceso");
                    break;
                    usleep(1000);
                case 0:
                    pause();
                    printf(" hijo2%d [%d]\n", i, getpid());
                    kill(getppid(), SIGUSR1);
                    break;
                default:
                    usleep(1000);
                    kill(pidhijo, SIGUSR1);
                    pause();
                    printf(" hijo%d [%d]\n", i + 1, getpid());
                    // wait(NULL);
                    kill(child[0], SIGUSR1);
                    // printf("%d \n",child[0]);
                    break;
                }  
            }
            else
            {
                pid_t pidhijo = fork();
                switch (pidhijo)
                {
                case -1:
                    error("Error en la creación del nuevo proceso");
                    break;
                case 0:
                    pause();
                    printf(" hijo1%d [%d]\n", i + 1, getpid());
                    kill(getppid(), SIGUSR1);
                    break;
                default:
                    usleep(1000);
                    kill(pidhijo, SIGUSR1);
                    pause();
                    //wait(NULL);
                    printf(" hijo%d [%d]\n", i + 1, getpid());
                    kill(root, SIGUSR1);
                    break;
                }
                break;
            }
            break;
        } 
           
        
    }

    if (signal(SIGUSR1, oldhandler) == SIG_ERR)
    {
        perror("signal: ");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}