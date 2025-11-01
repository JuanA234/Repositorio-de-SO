#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define NHIJOS 2
#define NFD 2

void error(char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}


void imprimirMatriz(int **matriz, int filas, int columnas){
    if (matriz == NULL)
    {
        printf("La matriz está vacía.\n");
        return;
    }

    for (int i = 0; i < filas; i++)
    {
        for (int j = 0; j < columnas; j++)
        {
            printf("%d ", matriz[i][j]);
        }
        printf("\n");
    }
}


int **leerArchivo(const char *nombreArchivo, int *filas, int *columnas){
    FILE *f = fopen(nombreArchivo, "r");
    if (!f)
        error("Error al abrir el archivo");

    // Leer número de filas y columnas
    if (fscanf(f, "%d", filas) != 1)
        error("Error al leer filas");
    if (fscanf(f, "%d", columnas) != 1)
        error("Error al leer columnas");

    // Reservar memoria para la matriz
    int **matriz = malloc((*filas) * sizeof(int *));
    if (!matriz)
        error("Error al asignar memoria para filas");

    for (int i = 0; i < *filas; i++)
    {
        matriz[i] = malloc((*columnas) * sizeof(int));
        if (!matriz[i])
            error("Error al asignar memoria para columnas");
    }

    // Leer los valores de la matriz
    for (int i = 0; i < *filas; i++)
    {
        for (int j = 0; j < *columnas; j++)
        {
            char c;
            if (fscanf(f, " %c", &c) != 1)
                error("Error al leer dato de matriz");
            matriz[i][j] = c - '0';
        }
    }

    fclose(f);
    return matriz;
}

int main(){

    int filas, columnas;
    int **matriz = leerArchivo("nidos.txt", &filas, &columnas);
 

    printf("Filas: %d\nColumnas: %d\n\n", filas, columnas);
    imprimirMatriz(matriz, filas, columnas);

    int fd[NFD][2];
    for (int i = 0; i < NFD; i++) {
        if (pipe(fd[i]) == -1) {
            error("Error creando el pipe");
        }
    }

    pid_t root = getpid();
    pid_t pidHijo;

    for(int i=0; i<NHIJOS; i++){
        pidHijo = fork(); 
        if(!pidHijo){
            pidHijo = i;
            break;
        }else if(pidHijo == -1){
            error("error en la creación del nuevo proceso");
        }
    }

    if(root = getpid()){
        for(int i=0; i<NFD; i++){
            close(fd[i][1]);
        }
    }else{
        for(int i=0; i<NFD; i++){
            close(fd[i][0]);
            if(i!=0){
                close(fd[i][1]);
            }
        }

    }

}