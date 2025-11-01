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

int *encontrarNidos(int **matriz, int filasTotal, int columnasTotal, int filaInicial, int filaFinal, int *parejas){
    int *coordenadas = NULL;
    int count = 0;
    for(int i=filaInicial; i<filaFinal; i++){
        for(int j=0; j<columnasTotal; j++){
            if(matriz[i][j]==1){
                int nidoEncontrado = 0;
                int filaVecino;
                int columnaVecino;
                for(filaVecino = -3; filaVecino<=3 && !nidoEncontrado; filaVecino++){
                    for(columnaVecino = -3; columnaVecino<=3 && !nidoEncontrado; columnaVecino++){
                        if(filaVecino==0 && columnaVecino==0){
                            continue;
                        }

                        int filaVecinoActual = i + filaVecino;
                        int columnaVecinoActual = j+columnaVecino;

                        if((filaVecinoActual>=0 && filaVecinoActual < filasTotal) && 
                        (columnaVecinoActual>=0 && columnaVecinoActual<columnasTotal)){
                            if(matriz[filaVecinoActual][columnaVecinoActual]==2){
                                coordenadas = realloc(coordenadas, (count+1)*2*sizeof(int));
                                if (!coordenadas)
                                    error("Error al asignar memoria para coordenadas"); 
                                
                                coordenadas[count*2]= i;
                                coordenadas[count*2 + 1]= j;
                                count++;
                                nidoEncontrado = 1;
                            }
                        }
                    }
                }
            }
        }
    }
    *parejas = count;
    return coordenadas;

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

    if(root == getpid()){
        for(int i=0; i<NFD; i++){
            close(fd[i][1]);
        }

        for (int i = 0; i < NHIJOS; i++) {
            wait(NULL);
        }

        for(int i=0; i<NFD; i++){
             int parejas = 0;
            ssize_t bytesLeidos = read(fd[i][0], &parejas, sizeof(int));
            if(bytesLeidos == 0){
                printf("Hijo %d no envio nada \n", i);
                close(fd[i][0]);
                continue;
            }else if(bytesLeidos != sizeof(int)){
                printf("Error de lectura en la tuberia #%d \n", i);
                close(fd[i][0]);
                continue;
            }

            printf("El hijo #%d envio %d coordenadas \n", i, parejas);
            if(parejas>0){
                int *coordenadas = malloc(parejas * 2 * sizeof(int));
                if(!coordenadas){
                    error("error al asignar memoria para coordenadas en el padre");
                }
                ssize_t bytesParaLeer = (ssize_t) parejas * 2 * sizeof(int);
       
                ssize_t lectura = read(fd[i][0], coordenadas, bytesParaLeer);
                if (lectura < 0) { 
                    error("error en la lectura de coordenadas del padre"); 
                }
    
                for (int k = 0; k < parejas; k++) {
                    printf("  (%d, %d)\n", coordenadas[k*2], coordenadas[k*2+1]);
                }
                free(coordenadas);

            }
        }

    }else{
        for(int i=0; i<NFD; i++){
            close(fd[i][0]);
            if(i!=pidHijo) close(fd[i][1]);
            
        }

        int mitadMatriz = filas / 2;

        int filaInicial = pidHijo*mitadMatriz;
        int filaFinal = (pidHijo==0) ? mitadMatriz: filas;

        int parejas = 0;
        int *coordenadas = encontrarNidos(matriz, filas, columnas, filaInicial, filaFinal, &parejas);

         if(write(fd[pidHijo][1], &parejas, sizeof(int)) != sizeof(int)){
            error("Error con el hijo al escribir el número de coordenadas encontradas \n");
        }else if(parejas>0){
            ssize_t bytesParaEscribir = (ssize_t)parejas * 2 *sizeof(int);
            ssize_t escritura = write(fd[pidHijo][1], coordenadas, bytesParaEscribir);
            if (escritura <= 0) { 
                error("error al escribir coordenadas"); 
            }
        }
        
        close(fd[pidHijo][1]);
        free(coordenadas);

    }

       // Liberar memoria
    for (int i = 0; i < filas; i++)
        free(matriz[i]);
    free(matriz);
}