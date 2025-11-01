#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define NFD 11


void error(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(){

    
    int n;

    system("clear");
    printf("Ingresar número de iteraciones: ");
    scanf("%d", &n);

    int fd[NFD][2];
    for (int i = 0; i < NFD; i++) {
        if (pipe(fd[i]) == -1) {
            error("Error creando el pipe");
        }
    }


    pid_t root = getpid();
    pid_t hijos[3];
    pid_t nieto;
    pid_t bisnieto;
    pid_t tataranietos[2];
    int iterador;
    char buffer;

    for(int i=0; i<3; i++){
        hijos[i] = fork();
        if(!hijos[i]){
            iterador=i;
            hijos[i] = getpid();
            if(i==1){
                nieto = fork();
                if(!nieto){
                    nieto = getpid();
                    bisnieto = fork();
                    if(!bisnieto){
                        bisnieto = getpid();
                        for(int j=0; j<2; j++){
                            tataranietos[j] = fork();
                            if(!tataranietos[j]){
                                iterador=j;
                                tataranietos[j] = getpid();
                                break;
                            }
                        }
                    }
                }
            }   
            break;
        } else if (hijos[i] == -1){
            error("Error en la creación del nuevo proceso");
        }
    }

        if(root==getpid()){//logica para padre
            
            for(int i=0; i<NFD; i++){
                if(i!=NFD-1) close(fd[i][0]);
                if(i!=0) close(fd[i][1]);
            }
            buffer = 'a';
            for(int i=0; i<n; i++){
                printf("------ Iteracion N° %d -----\n", i+1);
                printf("Proceso padre %d: %c\n", getpid(), buffer);
                buffer='d';
                write(fd[0][1], &buffer, sizeof(buffer));

                read(fd[NFD-1][0], &buffer, sizeof(buffer));
                printf("Proceso padre %d: %c\n", getpid(), buffer);

            }

            close(fd[0][1]);
            close(fd[NFD-1][0]);
            
        }else if(hijos[iterador]==getpid()){//logica para hijos de primer nivel
            if(iterador==0){
                for(int i=0; i<NFD; i++){
                    if(i!=0) close(fd[i][0]);
                    if(i!=1)close(fd[i][1]);
                } 

                for(int i=0; i<n; i++){     
                    read(fd[0][0], &buffer, sizeof(buffer));
                    printf("Proceso %d - padre [%d]: %c\n", getpid(), getppid(), buffer);
                    buffer='c';
                    write(fd[1][1], &buffer, sizeof(buffer));          
                }

                close(fd[0][0]);
                close(fd[1][1]);

            }else if(iterador==1){
                for(int i=0; i<NFD; i++){
                    if(i!=1 && i!=8) close(fd[i][0]);
                    if(i!=2 && i!=9) close(fd[i][1]);
                }  
                
                for(int i=0; i<n; i++){     
                    read(fd[1][0], &buffer, sizeof(buffer));
                    printf("Proceso %d - padre [%d]: %c\n", getpid(), getppid(), buffer);
                    buffer='e';
                    write(fd[2][1], &buffer, sizeof(buffer)); 

                    read(fd[8][0], &buffer, sizeof(buffer));
                    printf("Proceso %d - padre [%d]: %c\n", getpid(), getppid(), buffer);
                    buffer='b';
                    write(fd[9][1], &buffer, sizeof(buffer));          
                }
                close(fd[1][0]);
                close(fd[2][1]);
                close(fd[8][0]);
                close(fd[9][1]);
                
            }else{
                for(int i=0; i<NFD; i++){
                    if(i!=9) close(fd[i][0]);
                    if(i!=NFD-1) close(fd[i][1]);
                }
                for(int i=0; i<n; i++){     
                    read(fd[9][0], &buffer, sizeof(buffer));
                    printf("Proceso %d - padre [%d]: %c\n", getpid(), getppid(), buffer);
                    buffer='a';
                    write(fd[NFD-1][1], &buffer, sizeof(buffer));          
                }

                close(fd[0][9]);
                close(fd[NFD-1][1]);
            
            }
            
        }else{//logica para hijos de bajo nivel
           if(nieto==getpid()){
            for(int i=0; i<NFD; i++){
                if(i!=2 && i!=7) close(fd[i][0]);
                if(i!=3 && i!=8) close(fd[i][1]);
            } 
            
            for(int i=0; i<n; i++){     
                read(fd[2][0], &buffer, sizeof(buffer));
                printf("Proceso %d - padre [%d]: %c\n", getpid(), getppid(), buffer);
                buffer='f';
                write(fd[3][1], &buffer, sizeof(buffer)); 

                read(fd[7][0], &buffer, sizeof(buffer));
                printf("Proceso %d - padre [%d]: %c\n", getpid(), getppid(), buffer);
                buffer='c';
                write(fd[8][1], &buffer, sizeof(buffer));          
                }
                close(fd[2][0]);
                close(fd[3][1]);
                close(fd[7][0]);
                close(fd[8][1]);
                

           }else if(bisnieto==getpid()){
                for(int i=0; i<NFD; i++){
                    if(i!=3 && i!=6) close(fd[i][0]);
                    if(i!=4 && i!=7) close(fd[i][1]);
                } 
                for(int i=0; i<n; i++){     
                    read(fd[3][0], &buffer, sizeof(buffer));
                    printf("Proceso %d - padre [%d]: %c\n", getpid(), getppid(), buffer);
                    buffer='h';
                    write(fd[4][1], &buffer, sizeof(buffer)); 

                    read(fd[6][0], &buffer, sizeof(buffer));
                    printf("Proceso %d - padre [%d]: %c\n", getpid(), getppid(), buffer);
                    buffer='e';
                    write(fd[7][1], &buffer, sizeof(buffer));          
                }
                close(fd[3][0]);
                close(fd[4][1]);
                close(fd[6][0]);
                close(fd[7][1]);
           }else if(tataranietos[iterador]==getpid()){
                int indexRead = iterador + 4;
                int indexWrite = iterador +5;
                for(int i=0; i<NFD; i++){
                    if(i!=indexRead) close(fd[i][0]);
                    if(i!=indexWrite) close(fd[i][1]);
                }

                for(int i=0; i<n; i++){
                    read(fd[indexRead][0], &buffer, sizeof(buffer));
                    printf("Proceso %d - padre [%d]: %c\n", getpid(), getppid(), buffer);

                    buffer = (indexRead == 4) ? 'g':'f';
                    write(fd[indexWrite][1], &buffer, sizeof(buffer)); 

                }
                close(fd[indexRead][0]);
                close(fd[indexWrite][1]);
           }
        }
    

   
    return EXIT_SUCCESS;

}
