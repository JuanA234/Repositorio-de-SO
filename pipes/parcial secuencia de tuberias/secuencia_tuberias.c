#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#define EOL '\0'
char buff = 'a';

void error(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}


int main(){

    pid_t root = getpid();
    int hijo;

    //Creamos las tuberias, que para este caso son 9
    int fd[14][2];
      for (int i = 0; i < 14; i++) {
        if (pipe(fd[i]) == -1) {
            error("Error creando el pipe");
        }
    }

       for(hijo =0; hijo<3; hijo++){
        pid_t child = fork();
        if(!child){
            break;
        }else if(child == -1){
            error("error en la creación del nuevo proceso");
        }
    }

    if(root == getpid()){

          // Cierra todos los pipes que no usa
        for(int j=0; j<14; j++){
            if(j!=0) close(fd[j][1]);
            if(j!=13) close(fd[j][0]);
        }


        printf("Proceso padre %d [%d] = %c\n", getpid(), getppid(), buff);
        buff = 'd';
        write(fd[0][1], &buff, sizeof(buff));
        close(fd[0][1]);

        for(int i=0; i<3; i++) wait(NULL);

        read(fd[13][0], &buff, sizeof(buff));
        close(fd[13][0]);

       printf("Proceso padre %d [%d] = %c\n", getpid(), getppid(), buff);

    }else{
        if(hijo==0){
            int nieto;
            for(nieto =0; nieto<2; nieto++){
                pid_t child = fork();
                if(!child){
                    break;
                }else if(child == -1){
                    error("error en la creación del nuevo proceso");
                }
            }
            if(nieto==2){
                for(int i=0; i<14; i++){
                    if(i!=8 && i!=12) close(fd[i][0]);
                    if(i!=9 && i!=13) close(fd[i][1]);
                }

                read(fd[8][0], &buff, sizeof(buff));
                close(fd[8][0]);
                printf("Proceso %d [%d] = %c\n", getpid(), getppid(), buff);

                buff = 'e';
                write(fd[9][1], &buff, sizeof(buff));
                close(fd[9][1]);

                
                wait(NULL);

                read(fd[12][0], &buff, sizeof(buff));
                close(fd[12][0]);
                printf("Proceso %d [%d] = %c\n", getpid(), getppid(), buff);

                buff = 'a';
                write(fd[13][1], &buff, sizeof(buff));
                close(fd[13][1]);
            } else if(nieto == 0){
                for(int i=0; i<14; i++){
                    if(i!=9 && i!=11) close(fd[i][0]);
                    if(i!=10 && i!=12) close(fd[i][1]);
                }

                read(fd[9][0], &buff, sizeof(buff));
                close(fd[9][0]);
                printf("Proceso %d [%d] = %c\n", getpid(), getppid(), buff);

                buff = 'h';
                write(fd[10][1], &buff, sizeof(buff));
                close(fd[10][1]);

                read(fd[11][0], &buff, sizeof(buff));
                close(fd[11][0]);
                printf("Proceso %d [%d] = %c\n", getpid(), getppid(), buff);

                buff = 'b';
                write(fd[12][1], &buff, sizeof(buff));
                close(fd[12][1]);

            }else{
                 for(int i=0; i<14; i++){
                    if(i!=10) close(fd[i][0]);
                    if(i!=11) close(fd[i][1]);
                }
                read(fd[10][0], &buff, sizeof(buff));
                close(fd[10][0]);
                printf("Proceso %d [%d] = %c\n", getpid(), getppid(), buff);

                buff = 'e';
                write(fd[11][1], &buff, sizeof(buff));
                close(fd[11][1]);
            }

            exit(EXIT_SUCCESS);

        }else if(hijo==1){
            if(!fork()){
                for(int i=0; i<14; i++){
                    if(i!=6) close(fd[i][0]);
                    if(i!=7) close(fd[i][1]);
                }
                read(fd[6][0], &buff, sizeof(buff));
                close(fd[6][0]);
                printf("Proceso %d [%d] = %c\n", getpid(), getppid(), buff);

                buff = 'c';
                write(fd[7][1], &buff, sizeof(buff));
                close(fd[7][1]);

            }else{
                for(int i=0; i<14; i++){
                    if(i!=5 && i!=7) close(fd[i][0]);
                    if(i!=6 && i!=8) close(fd[i][1]);
            }

                read(fd[5][0], &buff, sizeof(buff));
                close(fd[5][0]);
                printf("Proceso %d [%d] = %c\n", getpid(), getppid(), buff);

                buff = 'f';
                write(fd[6][1], &buff, sizeof(buff));
                close(fd[6][1]);

                wait(NULL);

                read(fd[7][0], &buff, sizeof(buff));
                close(fd[7][0]);
                printf("Proceso %d [%d] = %c\n", getpid(), getppid(), buff);

                buff = 'b';
                write(fd[8][1], &buff, sizeof(buff));
                close(fd[8][1]);
            }



            exit(EXIT_SUCCESS);

        }else{
            int nieto;
            for(nieto =0; nieto<2; nieto++){
                pid_t child = fork();
                if(!child){
                    break;
                }else if(child == -1){
                    error("error en la creación del nuevo proceso");
                }
            }

            if(nieto==2){
                for(int i=0; i<14; i++){
                    if(i!=0 && i!=4) close(fd[i][0]);
                    if(i!=1 && i!=5) close(fd[i][1]);
                }
   
                read(fd[0][0], &buff, sizeof(buff));
                close(fd[0][0]);
                printf("Proceso %d [%d] = %c\n", getpid(), getppid(), buff);

                buff = 'g';
                write(fd[1][1], &buff, sizeof(buff));
                close(fd[1][1]);


                wait(NULL);

                read(fd[4][0], &buff, sizeof(buff));
                close(fd[4][0]);
                printf("Proceso %d [%d] = %c\n", getpid(), getppid(), buff);

                buff = 'c';
                write(fd[5][1], &buff, sizeof(buff));
                close(fd[5][1]);
            }
             else if(nieto == 0){
                for(int i=0; i<14; i++){
                     if(i!=1 && i!=3) close(fd[i][0]);
                    if(i!=2 && i!=4) close(fd[i][1]);
                }

                read(fd[1][0], &buff, sizeof(buff));
                close(fd[1][0]);
                printf("Proceso %d [%d] = %c\n", getpid(), getppid(), buff);

                buff = 'i';
                write(fd[2][1], &buff, sizeof(buff));
                close(fd[2][1]);

                read(fd[3][0], &buff, sizeof(buff));
                close(fd[3][0]);
                printf("Proceso %d [%d] = %c\n", getpid(), getppid(), buff);

                buff = 'd';
                write(fd[4][1], &buff, sizeof(buff));
                close(fd[4][1]);

            }else{
                 for(int i=0; i<14; i++){
                    if(i!=2) close(fd[i][0]);
                    if(i!=3) close(fd[i][1]);
                }
                read(fd[2][0], &buff, sizeof(buff));
                close(fd[2][0]);
                printf("Proceso %d [%d] = %c\n", getpid(), getppid(), buff);

                buff = 'g';
                write(fd[3][1], &buff, sizeof(buff));
                close(fd[3][1]);
            }

            

            exit(EXIT_SUCCESS);
        }


    }

  
    return EXIT_SUCCESS;
}