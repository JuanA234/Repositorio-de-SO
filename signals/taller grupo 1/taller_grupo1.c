#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h> 

int contador = 0;
int numero;

void *oldhandler;
void *oldhandler2;


void signalHandler(int sig) {
    if (sig == SIGUSR1) {
        contador++;
    } else if (sig == SIGUSR2) {
        numero = contador;
        printf("%d ", numero);
        fflush(stdout);
        contador = 0;
    }
}

void error(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int **leerArchivo(const char *nombreArchivo, int *filas, int *columnas)
{
    FILE *f = fopen(nombreArchivo, "r");
    if (!f)
        error("Error al abrir el archivo");

    
    char linea[50];
    

    // Leer número de filas y columnas
    
   if(fscanf(f, "%d,%d", filas, columnas) != 2) {
        error("Error al leer el las filas y columnas");
   }

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

    oldhandler = signal(SIGUSR1, signalHandler);
    oldhandler2 = signal(SIGUSR2, signalHandler);

    if (oldhandler == SIG_ERR){
        error("signal:");
    }

     if (oldhandler2 == SIG_ERR){
        error("signal:");
    }

    int root = getpid();

    pid_t pidHijo = fork();

    if(root==getpid()){

    int filas, columnas; 
    int **matriz = leerArchivo("matriz.txt", &filas, &columnas);

    if (matriz == NULL){
            printf("La matriz está vacía.\n");
        }
        for (int i = 0; i < filas; i++){
            for (int j = 0; j < columnas; j++){
                for(int k=0; k<matriz[i][j]; k++){
                    kill(pidHijo, SIGUSR1);
                    usleep(10000);
                    
                }
                kill(pidHijo, SIGUSR2);
                usleep(10000);
                    
            } 
            printf("\n");
        }
    
    }else{
        while (1){
            pause();
            //printf("%d\n", numero);
        }
          
          
    }

  

    if (signal(SIGUSR1, oldhandler) == SIG_ERR){
        perror("signal: ");
        exit(EXIT_FAILURE);
    }

    
    if (signal(SIGUSR2, oldhandler2) == SIG_ERR){
        perror("signal: ");
        exit(EXIT_FAILURE);
    }


 
    
    return EXIT_SUCCESS;
}