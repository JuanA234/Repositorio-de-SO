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
    pid_t nietos1[2];
    pid_t nietos2[2];
    pid_t bisnieto;
    int i;

    for (i = 0; i < 2; i++){
        child[i] = fork();
        if (!child[i]){
            if(i==0){
                child[i] = getpid();
                for(int j=0; j<2; j++){
                    nietos1[j] = fork();
                    if(!nietos1[j]){
                        nietos1[j] = getpid();
                        if(j==0){
                            bisnieto = fork();
                            if(!bisnieto){
                                bisnieto = getpid();
                            }
                        }
                        break;
                    }else if (nietos1[j] == -1){
                        error("Error en la creación del nuevo proceso");
                    } 
                }
            }else if(i==1){
                child[i] = getpid();
                for(int k=0; k<2; k++){
                     nietos2[k] = fork();
                     if(!nietos2[k]){
                        nietos2[k] = getpid();
                        break;
                     }else if (nietos2[k] == -1){
                        error("Error en la creación del nuevo proceso");
                    } 

                }
            }
            break;
        }
        else if (child[i] == -1){
            error("Error en la creación del nuevo proceso");
        }
    }


    for (int j = 0; j < n; j++){
        if(i==2){ // proceso padre
            usleep(1000);
            kill(child[1], SIGUSR1);
            pause();
            printf("Padre [%d] \n", getpid());

        }
        else{// logica para procesos hijos
            if (i == 1){
                if(child[1] == getpid()){
                    pause();
                    printf(" hijo%d [%d]\n", i + 1, getpid());
                    usleep(1000);
                    kill(nietos2[1], SIGUSR1);
                    pause();
                    printf(" hijo%d [%d]\n", i + 1, getpid());
                    kill(nietos2[0], SIGUSR1);
                    pause();
                    printf(" hijo%d [%d]\n", i + 1, getpid());
                    // wait(NULL);
                    kill(child[0], SIGUSR1);
                    // printf("%d \n",child[0]);
                }else{
                    if(nietos2[1]==getpid()){
                        pause();
                        printf(" hijo2%d [%d]\n", i+1, getpid());
                        kill(getppid(), SIGUSR1);
                    }else if(nietos2[0]==getpid()){
                        pause();
                        printf(" hijo2%d [%d]\n", i, getpid());
                        kill(getppid(), SIGUSR1);
                    }
                    
                }
                
            }
            else{
                //printf("%s \n", "AQUI");
                if(child[0] == getpid()){
                    pause();
            printf(" hijo%d [%d]\n", i + 1, getpid());
                    usleep(1000);
                    kill(nietos1[1], SIGUSR1);
                    pause();
                    printf(" hijo%d [%d]\n", i + 1, getpid());
                    kill(nietos1[0], SIGUSR1);
                    pause();
                    printf(" hijo%d [%d]\n", i + 1, getpid());
                    // wait(NULL);
                    kill(getppid(), SIGUSR1);
                    // printf("%d \n",child[0]);
                }else{
                    if(nietos1[0]==getpid()){
                        pause();
                        printf(" hijo1%d [%d]\n", i+1, getpid());
                       
                        usleep(1000);
                    
                        kill(bisnieto, SIGUSR1);
                        pause();
                        printf(" hijo1%d [%d]\n", i+1, getpid());
                        kill(getppid(), SIGUSR1);
                        
                        
                    }else if(nietos1[1]==getpid()){
                        pause();
                        printf(" hijo1%d [%d]\n", i+2, getpid());
                        kill(getppid(), SIGUSR1);
                    }else if(bisnieto==getpid()){
                            pause();
                            printf(" hijo11%d [%d]\n", i+2, getpid());
                            kill(getppid(), SIGUSR1);
                    }
                    
                }
            }
        } 
           
        
    }

    if (signal(SIGUSR1, oldhandler) == SIG_ERR)
    {
        perror("signal: ");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}