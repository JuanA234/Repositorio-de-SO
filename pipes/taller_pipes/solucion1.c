#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>

int main(){
    system("clear");
    int i, j, k, id, n, dato=0, fd[11][2];
    pid_t childs[3], root = getpid();

    printf("Ingrese el numero de iteraciones: ");
    scanf("%d", &n);

    if (n < 1){
        printf("Valor invalido.\n"); exit(-1);
    }

    for (int t=0; t<11; t++) if ( pipe(fd[t]) ) exit(-1);

    id = 1;
    for (i=0; i<3; i++){
        childs[i] = fork();
        if (childs[i] == 0){
            id = (id*10)+i;
            if (i == 1){
                for (j=0; j<2; j++){
                    childs[0] = fork();
                    if (childs[0] != 0){ // si soy padre
                        break;
                    }else{ // si soy hijo
                        id = (id*10)+j;
                    }
                }
                if (j == 2){
                    for (k=0; k<2; k++){
                        childs[k] = fork();
                        if (childs[k] == 0){
                            id = (id*10)+k;
                            break;
                        }
                    }
                }
            }
            break;
        }
    }

    if(root == getpid()){
        char b[500];
        sprintf(b,"pstree -lp %d",getpid());
        system(b);
    }else sleep(3);

    if (id == 1){ //padre
        for (int a=0; a<11; a++){
            if (a != 10) close(fd[a][0]);
            if (a != 0) close(fd[a][1]);
        }
        printf("\n");
        for (int r=0; r<n; r++){
            printf("------ Iteracion N° %d -----\n", r+1);
            printf("Proceso padre %d\n", getpid());
            write(fd[0][1], &dato, sizeof(int));

            read(fd[10][0], &dato, sizeof(int));
            printf("Proceso padre %d\n", getpid());
        }

        close(fd[10][0]);    close(fd[0][1]);
    }else{ //hijos
        if (id == 12){ // hijo 3
            for (int a=0; a<11; a++){
                if (a != 0) close(fd[a][0]);
                if (a != 1) close(fd[a][1]);
            }

            for (int r=0; r<n; r++){   
                read(fd[0][0], &dato, sizeof(int));
                printf("Proceso %d - padre [%d]\n", getpid(), getppid());
                write(fd[1][1], &dato, sizeof(int));
            }

            close(fd[0][0]);    close(fd[1][1]);
        }
        if (i ==  1){ // hijo 2 y sus decendientes
            if (id == 11){
                for (int a=0; a<11; a++){
                    if (a != 1 && a != 8) close(fd[a][0]);
                    if (a != 2 && a != 9) close(fd[a][1]);
                }

                for (int r=0; r<n; r++){
                    read(fd[1][0], &dato, sizeof(int));
                    printf("Proceso %d - padre [%d]\n", getpid(), getppid());
                    write(fd[2][1], &dato, sizeof(int));

                    read(fd[8][0], &dato, sizeof(int));
                    printf("Proceso %d - padre [%d]\n", getpid(), getppid());
                    write(fd[9][1], &dato, sizeof(int));
                }

                close(fd[1][0]);    close(fd[2][1]);
                close(fd[8][0]);    close(fd[9][1]);
            }
            if (id == 110){
                for (int a=0; a<11; a++){
                    if (a != 2 && a != 7) close(fd[a][0]);
                    if (a != 3 && a != 8) close(fd[a][1]);
                }

                for (int r=0; r<n; r++){
                    read(fd[2][0], &dato, sizeof(int));
                    printf("Proceso %d - padre [%d]\n", getpid(), getppid());
                    write(fd[3][1], &dato, sizeof(int));

                    read(fd[7][0], &dato, sizeof(int));
                    printf("Proceso %d - padre [%d]\n", getpid(), getppid());
                    write(fd[8][1], &dato, sizeof(int));
                }

                close(fd[2][0]);    close(fd[3][1]);
                close(fd[7][0]);    close(fd[8][1]);
            }
            if (id == 1101){
                for (int a=0; a<11; a++){
                    if (a != 3 && a != 6) close(fd[a][0]);
                    if (a != 4 && a != 7) close(fd[a][1]);
                }

                for (int r=0; r<n; r++){
                    read(fd[3][0], &dato, sizeof(int));
                    printf("Proceso %d - padre [%d]\n", getpid(), getppid());
                    write(fd[4][1], &dato, sizeof(int));

                    read(fd[6][0], &dato, sizeof(int));
                    printf("Proceso %d - padre [%d]\n", getpid(), getppid());
                    write(fd[7][1], &dato, sizeof(int));
                }

                close(fd[3][0]);    close(fd[4][1]);
                close(fd[6][0]);    close(fd[7][1]);
            }
            if (id == 11011){
                for (int a=0; a<11; a++){
                    if (a != 4) close(fd[a][0]);
                    if (a != 5) close(fd[a][1]);
                }

                for (int r=0; r<n; r++){
                    read(fd[4][0], &dato, sizeof(int));
                    printf("Proceso %d - padre [%d]\n", getpid(), getppid());
                    write(fd[5][1], &dato, sizeof(int));
                }

                close(fd[4][0]);    close(fd[5][1]);
            }
            if (id == 11010){
                for (int a=0; a<11; a++){
                    if (a != 5) close(fd[a][0]);
                    if (a != 6) close(fd[a][1]);
                }

                for (int r=0; r<n; r++){
                    read(fd[5][0], &dato, sizeof(int));
                    printf("Proceso %d - padre [%d]\n", getpid(), getppid());
                    write(fd[6][1], &dato, sizeof(int));
                }

                close(fd[5][0]);    close(fd[6][1]);
            }
        }
        if (id == 10 ){
            for (int a=0; a<11; a++){
                if (a != 9) close(fd[a][0]);
                if (a != 10) close(fd[a][1]);
            }

            for (int r=0; r<n; r++){
                read(fd[9][0], &dato, sizeof(int));
                printf("Proceso %d - padre [%d]\n", getpid(), getppid());
                write(fd[10][1], &dato, sizeof(int));
            }

            close(fd[9][0]);    close(fd[10][1]);
        }
    }

    // printf("[%d] id=%d \t\ti=%d \tj=%d \tk=%d\n", getpid(), id,i,j,k);

    return 0;
}
