#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/stat.h>


void error(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}


// Devuelve el tamaño total necesario para la matriz
unsigned int sizeof_dm(int rows, int cols, size_t sizeElement) {
    size_t size = 0;
    size += rows * sizeof(void *);              // espacio para los índices
    size += rows * cols * sizeElement;          // espacio para los datos
    return size;
}

// Configura los punteros internos dentro del bloque de memoria continua
void create_index(void **m, int rows, int cols, size_t sizeElement) {
    int i;
    size_t sizeRow = cols * sizeElement;
    m[0] = m + rows;                 // los datos comienzan justo después del índice
    for (i = 1; i < rows; i++)
        m[i] = (m[i - 1]) + sizeRow; // cada fila apunta a la siguiente
}

void imprimirMatriz(int **matriz, int filas, int columnas){
    if (matriz == NULL){
        printf("La matriz está vacía.\n");
        return;
    }
    for (int i = 0; i < filas; i++){
        for (int j = 0; j < columnas; j++){
            printf("%d ", matriz[i][j]);
        }
        printf("\n");
    }
}

int **leerArchivo(const char *nombreArchivo, int *filas, int *columnas, int *ciclosDeSimulacion
    , int *numeroDeHijos)
{
    FILE *f = fopen(nombreArchivo, "r");
    if (!f)
        error("Error al abrir el archivo");

    // Leer número de ciclos de simulación (horas) y de hijos
    if (fscanf(f, "%d", ciclosDeSimulacion) != 1)
        error("Error al leer horas");
    if (fscanf(f, "%d", numeroDeHijos) != 1)
        error("Error al leer el numero de hijos");

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

// Asigna a cada hijo un rango [inicio, fin] de índices del bosque
void obtenerRangoTrabajo(int hijo_id, int hijos, int total, int *inicio, int *fin) {
    int base = total / hijos; //Cantidad base por hijo
    int resto = total % hijos; //Las celdas que sobren

    *inicio = hijo_id * base + (hijo_id < resto ? hijo_id : resto);
    *fin = *inicio + base - 1;
    if (hijo_id < resto) (*fin)++;

    if (*inicio >= total) *inicio = *fin = -1; // sin trabajo
}


void faseDePropagacion(int **matriz,  int **nuevaMatriz, int inicio, int final, int filasTotal, int columnasTotal){
     if (inicio == -1 || final == -1) return;

    // Crear una copia local de la matriz original (estado al inicio de la fase)
    int copia[filasTotal][columnasTotal];
    for (int i = 0; i < filasTotal; i++)
        for (int j = 0; j < columnasTotal; j++)
            copia[i][j] = matriz[i][j];

    for(int i = inicio; i<=final; i++){

        int fila = i/columnasTotal;
        int columna = i%columnasTotal;

        nuevaMatriz[fila][columna] = copia[fila][columna];


        if(copia[fila][columna]==0){

            int alreadyBurning = 0;
            int count=0;
            for(int j=-1; j<=1 && !alreadyBurning; j++){
                for(int k=-1; k<=1 && !alreadyBurning; k++){
                    if(k == 0 && j == 0){
                        continue;
                    }
                    int filaActual = fila  + j;
                    int columnaActual = columna +k;

                    // Verificamos que no se salga de los límites
                    if( (filaActual) >= 0 && (filaActual) < filasTotal &&
                     (columnaActual) >= 0 && (columnaActual) < columnasTotal){
                        if(copia[filaActual][columnaActual]==1){
                            count++;
                        }
                        if(count >= 2){
                            nuevaMatriz[fila][columna] = 1;
                            alreadyBurning = 1;
                        }
                    }
                }

            }
        }
    }
}

void faseDeConsumo(int **matriz, int inicio, int final, int filasTotal, int columnasTotal){
    if (inicio == -1 || final == -1) return;
    for(int i = inicio; i<=final; i++){

        int fila = i/columnasTotal;
        int columna = i%columnasTotal;

        if(matriz[fila][columna]==1){
            matriz[fila][columna] = 2;
        }
    }
}

/*
// Función que simula el trabajo de un hijo en una fase, la eliminaremos despues
void trabajar_en_fase(int id, int fase) {
    printf("Hijo %d trabajando en fase %d\n", id, fase);
    usleep(100000 * (id + 1)); // Simula tiempo de trabajo
}

*/

typedef struct {
    int fase_actual;     // Fase en la que estamos
    int hijos_listos;    // Cuántos hijos han terminado la fase
} Control;

int main(){

    int filas, columnas, ciclosDeSimulacion, numeroDeHijos;
    int **matrizLocal = leerArchivo("matriz_de_propagacion.txt", &filas, &columnas, 
        &ciclosDeSimulacion, &numeroDeHijos);

    printf("Información de la matriz: \n\n");

    printf("Horas: %d \nHijos: %d\nfilas: %d\ncolumnas: %d\n", ciclosDeSimulacion, numeroDeHijos,
    filas, columnas);
    printf("\n");
    imprimirMatriz(matrizLocal, filas, columnas);
    printf("\n");

    //Calcular tamaño del segmento
    size_t sizeMatrix = sizeof_dm(filas, columnas, sizeof(int));

    // Crear dos segmentos de memoria compartida: matriz (actual) y nuevaMatriz (resultados)
    int shmidMatriz = shmget(IPC_PRIVATE, sizeMatrix, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (shmidMatriz == -1)
        error("Error en shmget");
    int shmidNuevaMatriz = shmget(IPC_PRIVATE, sizeMatrix, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (shmidMatriz == -1)
        error("Error en shmget");

    //Mapear segmento
    int **matriz = (int **)shmat(shmidMatriz, 0, 0);
    int **nuevaMatriz = (int **)shmat(shmidNuevaMatriz, NULL, 0);

    //Configurar estructura de índices dentro del bloque compartido
    create_index((void *)matriz, filas, columnas, sizeof(int));
    create_index((void **)nuevaMatriz, filas, columnas, sizeof(int));

    //Copiar datos leídos a la matriz compartida
      for (int i = 0; i < filas; i++){
        for (int j = 0; j < columnas; j++){
            matriz[i][j] = matrizLocal[i][j];
            nuevaMatriz[i][j] = matrizLocal[i][j]; // inicialmente igual
        }
       
      }

    // Crear segmento de memoria compartida para la estructura Control
    int shmidControl = shmget(IPC_PRIVATE, sizeof(Control), IPC_CREAT | S_IRUSR | S_IWUSR);
    if (shmidControl == -1) {
       error("Error en shmget");
    }

    Control *ctrl = (Control *)shmat(shmidControl, 0, 0);
    ctrl->fase_actual = 0;
    ctrl->hijos_listos = 0;


    //Se crean los hijos
    int root = getpid();
    pid_t pidHijo;

    for(int i=0; i<numeroDeHijos; i++){
        pidHijo = fork();
        if(!pidHijo){
            pidHijo = i;
            break;
        }else if(pidHijo == -1){
            error("error en la creación del nuevo proceso");
        }

    }

    for(int ciclo=0; ciclo<ciclosDeSimulacion; ciclo++){
        if(root == getpid()){

            ctrl->fase_actual = 0;
            while (ctrl->hijos_listos < numeroDeHijos)
                usleep(1000);
            ctrl->hijos_listos = 0;

            ctrl->fase_actual = 1;
            while (ctrl->hijos_listos < numeroDeHijos)
                usleep(1000);
            ctrl->hijos_listos = 0;

            
            for (int i = 0; i < filas; i++) {
                for (int j = 0; j < columnas; j++) {
                    if (matriz[i][j] == 1) {
                        // Si estaba quemándose, pasa a consumido
                        matriz[i][j] = 2;
                    } 
                    else if (nuevaMatriz[i][j] == 1 && matriz[i][j] != 2) {
                        // Si debe comenzar a quemarse y aún no está consumido
                        matriz[i][j] = 1;
                }
            }
        }

    
            printf("Ciclo número :%d\n\n",ciclo+1);
            imprimirMatriz(matriz, filas, columnas);
            printf("\n");

        }else{
            while(1){
                int fase = ctrl->fase_actual;
                if (fase < 0) break;

                int celdaInicial;
                int celdaFinal;


                obtenerRangoTrabajo(pidHijo, numeroDeHijos, filas*columnas, &celdaInicial, &celdaFinal);
                if(fase == 0){
                    faseDePropagacion(matriz,nuevaMatriz, celdaInicial, celdaFinal, filas, columnas);
                }else if(fase == 1){
                    faseDeConsumo(matriz, celdaInicial, celdaFinal, filas, columnas);
                }
                 

                // Incrementar contador de hijos que terminaron
                __sync_fetch_and_add(&ctrl->hijos_listos, 1);


                //printf("Hijo %d terminó fase %d\n", pidHijo, fase);
                
                // Esperar hasta que el padre cambie de fase
                while (ctrl->fase_actual == fase) {
                    usleep(1000);
                }

                
            }

               
            // Desasociar la memoria compartida
            shmdt(ctrl);
            shmdt(matriz);
            shmdt(nuevaMatriz);
            exit(EXIT_SUCCESS);
        }
    
    }

    
    // Terminar hijos (señal de fin)
    ctrl->fase_actual = -1;

    // Esperar hijos solo al final
     for (int i = 0; i < numeroDeHijos; i++)
        wait(NULL);

    
     // Desasociar los segmentos
    shmdt(ctrl);
    shmdt(matriz);
    shmdt(nuevaMatriz);
    
    // Eliminar los segmentos de memoria
    shmctl(shmidMatriz, IPC_RMID, NULL);
    shmctl(shmidControl, IPC_RMID, NULL);
    shmctl(shmidNuevaMatriz, IPC_RMID, NULL);

     // liberar memoria local usada para lectura inicial
    for (int i = 0; i < filas; i++) free(matrizLocal[i]);
    free(matrizLocal);
 

    return EXIT_SUCCESS;
}