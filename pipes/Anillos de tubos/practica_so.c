#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#define EOL '\0'
#define MAX_READ 256
char buff[MAX_READ];



void error(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(){


    pid_t root = getpid();
    int nHijos;
    printf("Escriba la cantidad de hijos \n");
    scanf("%d", &nHijos);
    getchar(); 

    int i;

     // Creamos N+1 pipes (para conectar circularmente todos los procesos)
    int fd[nHijos+1][2];
    for (int i = 0; i <= nHijos; i++) {
        if (pipe(fd[i]) == -1) {
            error("Error creando el pipe");
        }
    }

    for(i=0; i<nHijos; i++){
        pid_t child = fork();
        if(!child){
            break;
        }else if(child == -1){
            error("error en la creación del nuevo proceso");
        }
    }

    if(root==getpid()){
        
        for (int j = 0; j <= nHijos; j++) {
            if (j != 0) close(fd[j][1]);      // Solo deja fd[0][1] abierto para escribir
            if (j != nHijos) close(fd[j][0]); // Solo deja fd[nHijos][0] abierto para leer
        }

        while(1){
            printf("Escriba el mensaje que quiera enviar: \n");
            fgets(buff, MAX_READ, stdin);
            if(strlen(buff)>1){
                buff[strlen(buff) - 1] ='\0';
                printf("[%d]write-->:%s\n\n", getpid(), buff);
                write(fd[0][1], buff, strlen(buff));
                break;

            }else{
                printf("Cadena muy corta \n");
            }
        }
        close(fd[0][1]);
       for (int i = 0; i < nHijos; i++) wait(NULL);

          // Leer el mensaje final desde el último pipe
       read(fd[nHijos][0], buff, MAX_READ);
       close(fd[nHijos][0]);

       printf("[%d]read<--:%s\n", getpid(), buff);

    }else{
        //Cerrar pipes no usados
        for(int j=0; j<=nHijos; j++){
            if (j != i) close(fd[j][0]);   
            if (j != i + 1) close(fd[j][1]); 
        }

        read(fd[i][0], buff, MAX_READ);
        close(fd[i][0]);
        printf("[%d]read<--:%s\n", getpid(), buff);

        char temp[50];
        sprintf(temp, " -> Hijo%d(pid=%d)", i + 1, getpid());
        strcat(buff, temp);

        write(fd[i+1][1], buff, strlen(buff)+1);
        close(fd[i+1][1]);


    }

    return EXIT_SUCCESS;
}