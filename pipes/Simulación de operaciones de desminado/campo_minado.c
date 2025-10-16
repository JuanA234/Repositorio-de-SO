#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>


void error(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

void imprimirMatriz(int **matriz, int filas, int columnas)
{
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

int **leerArchivo(const char *nombreArchivo, int *filas, int *columnas)
{
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

int *encontrarMinas(int **matriz,int filasTotal, int columnasTotal,
     int filaInicial, int filaFinal, int columnaInicial, int columnaFinal, int *parejas){

    int *coordenadas = NULL;
    //Variable para contar parejas
    int count = 0;

    for (int i = filaInicial; i < filaFinal; i++){
        for (int j = columnaInicial; j < columnaFinal; j++){
            if(matriz[i][j]==2){

                coordenadas = realloc(coordenadas, (count+1)*2*sizeof(int));
                if (!coordenadas)
                    error("Error al asignar memoria para coordenadas");  
        
                coordenadas[count*2]= i;
                coordenadas[count*2 + 1]= j;
                count++;
            }else if(matriz[i][j]==1){
                int found = 0;
                for(int k = -1; k<=1 && !found; k++){
                    for(int c = -1; c<=1 && !found; c++){
                        if(k == 0 && c == 0){
                            continue;
                        }
                         // Verificamos que no se salga de los límites
                         if( (i+k) >= 0 && (i+k) < filasTotal && (j+c) >= 0 && (j+c) < columnasTotal){
                            if(matriz[i+k][j+c] == 2){
                                coordenadas = realloc(coordenadas, (count+1)*2*sizeof(int *));
                                if (!coordenadas)
                                    error("Error al asignar memoria para coordenadas");  
        
                                coordenadas[count*2]= i;
                                coordenadas[count*2 + 1]= j;
                                count++;
                                found = 1; //Evitar duplicados
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
 

int main()
{
    int filas, columnas;
    int **matriz = leerArchivo("campo_minado_matriz.txt", &filas, &columnas);
 

    printf("Filas: %d\nColumnas: %d\n\n", filas, columnas);
    imprimirMatriz(matriz, filas, columnas);

    int columnasCuadrante = columnas/2, filasCuadrante = filas/2;
    int sobranteColumnas = columnas%2, filasSobrantes = filasCuadrante%2;

    const int TAMANIOFD = 4;
    int fd[TAMANIOFD][2];
        for (int i = 0; i < TAMANIOFD; i++) {
        if (pipe(fd[i]) == -1) {
            error("Error creando el pipe");
        }
    }

    
    pid_t root = getpid();
    int hijo;

    for(hijo=0; hijo<=3; hijo++){
     pid_t child = fork(); 
        if(!child){
            break;
        }else if(child == -1){
             error("error en la creación del nuevo proceso");
        }

    }

    if(root==getpid()){

        for(int i=0; i<TAMANIOFD; i++){
            close(fd[i][1]);
        }
  
        for (int i = 0; i < 4; i++) {
            wait(NULL);
        }
   
        printf("Proceso padre \n");

        //leer primero el número de parejas que envio cada hijo
        for(int i=0; i<TAMANIOFD; i++){
            int parejas = 0;
            //solo se van a leer los 4 primeros bytes que mando el hijo, es decir el conteo de las parejas
            ssize_t bytesLeidos = read(fd[i][0], &parejas, sizeof(int));
            if(bytesLeidos == 0){
                //el hijo no envio nada
                printf("Hijo %d no envio nada \n", i);
                close(fd[i][0]);
                continue;
            }else if(bytesLeidos != sizeof(int)){
                printf("Error de lectura en la tuberia #%d \n", i);
                close(fd[i][0]);
                continue;
            }

            //Ahora vamos a leer las coordenadas enviadas por el hijo
            printf("El hijo #%d envio %d coordenadas \n", i, parejas);
            if(parejas>0){
                int *coordenadas = malloc(parejas * 2 * sizeof(int));
                if(!coordenadas){
                    error("error al asignar memoria para coordenadas en el padre");
                }
                ssize_t bytesParaLeer = (ssize_t) parejas * 2 * sizeof(int);
                ssize_t bytesLeidos = 0;
                char *buffer = (char*)coordenadas;
                while (bytesLeidos < bytesParaLeer){
                    ssize_t lectura = read(fd[i][0], buffer + bytesLeidos, bytesParaLeer - bytesLeidos);
                    if (lectura < 0) { 
                        error("error en la lectura de coordenadas del padre"); 
                    }
                    if (lectura == 0) {
                    // El hijo cerró el pipe (fin de datos)
                    printf("EOF al leer coordenadas del hijo #%d (faltaban %zd bytes)\n", i, bytesParaLeer - bytesLeidos);
                     break;
                    }
                    bytesLeidos += lectura;    
                }

                // imprimir coordenadas
                for (int k = 0; k < parejas; k++) {
                    printf("  (%d, %d)\n", coordenadas[k*2], coordenadas[k*2+1]);
                }
                free(coordenadas);

            }
        }

    

    }else{

         printf("Proceso hijo #%d \n\n", hijo);
        for(int i=0; i<TAMANIOFD; i++){
            close(fd[i][0]);
            if(i!=hijo){
                close(fd[i][1]);
            }
        }

        int filaInicial = (hijo / 2)*(filasCuadrante);
        printf("Fila inical:%d \n", filaInicial);
        int filaFinal = (hijo==0 || hijo==1) ? filasCuadrante:filas; 
        printf("Final final:%d \n", filaFinal);

        int  columnaInicial = (hijo % 2)*(columnasCuadrante);
        printf("Columna inicial:%d \n", columnaInicial);
        int columnaFinal = (hijo==0 || hijo==2) ? columnasCuadrante:columnas; 
        printf("Columna final:%d  \n\n", columnaFinal);

        int parejas = 0;
        int *coordenadas = encontrarMinas(matriz, filas, columnas, filaInicial, filaFinal, columnaInicial, columnaFinal,
        &parejas);

        if(write(fd[hijo][1], &parejas, sizeof(int)) != sizeof(int)){
            error("Error con el hijo %d, al escribir el número de coordenadas encontradas \n");
        }else if(parejas > 0){
            ssize_t bytesParaEscribir = (ssize_t)parejas * 2 *sizeof(int);
            ssize_t bytesEscritos = 0;
            char *buffer = (char*)coordenadas;
            while (bytesEscritos < bytesParaEscribir){
                ssize_t escritura = write(fd[hijo][1], buffer + bytesEscritos, bytesParaEscribir - bytesEscritos);
                if (escritura <= 0) { 
                    error("error al escribir coordenadas"); 
                }
                bytesEscritos += escritura;
            }
            
        }

        close(fd[hijo][1]);
        free(coordenadas);
       
    }

    // Liberar memoria
    for (int i = 0; i < filas; i++)
        free(matriz[i]);
    free(matriz);

    return EXIT_SUCCESS;
}
