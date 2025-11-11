#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <ctype.h>
#include <regex.h>

#define NHIJOS 2
#define NFD 2
#define TAM_BUFFER 256


void error(char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

void imprimirCrucigrama(char **crucigrama, int filas, int columnas){
    if (crucigrama == NULL)
    {
        printf("La matriz está vacía.\n");
        return;
    }

    for (int i = 0; i < filas; i++)
    {
        for (int j = 0; j < columnas; j++)
        {
            printf("%c", crucigrama[i][j]);
        }
        printf("\n");
    }
}

void leerArchivo(char *nombreArchivo,int *filas, int *columnas, char ***crucigrama, int *nPalabras, char ***palabras){
    FILE *f = fopen(nombreArchivo, "r");
    if (!f)
        error("Error al abrir el archivo");

    if (fscanf(f, "%d", filas) != 1)
        error("Error al leer filas");
    if (fscanf(f, "%d", columnas) != 1)
        error("Error al leer columnas");

    *crucigrama = malloc((*filas) * sizeof(char *));
    if (!*crucigrama)
        error("Error al asignar memoria para filas");

    for (int i = 0; i < *filas; i++)
    {
        (*crucigrama)[i] = malloc((*columnas) * sizeof(char));
        if (!(*crucigrama)[i])
            error("Error al asignar memoria para columnas");
    }

    for (int i = 0; i < *filas; i++)
    {
        for (int j = 0; j < *columnas; j++)
        {
            char c;
            if (fscanf(f, " %c", &c) != 1)
                error("Error al leer dato del crucigrama");
            (*crucigrama)[i][j] = c;
        }
    }

    if (fscanf(f, "%d\n", nPalabras) != 1)
        error("Error al leer numero de palabras a buscar");



    *palabras = malloc (*nPalabras * sizeof(char*));
    if (!*palabras) {
        error("Error al asignar memoria para las palabras");
    }

    char *linea = NULL; 
    size_t len = 0;
    ssize_t leidos; 
    int count = 0; 

    while(count < *nPalabras && (leidos = getline(&linea, &len, f)) != -1){
        if (linea[leidos - 1] == '\n')
            linea[leidos - 1] = '\0';
        (*palabras)[count] = strdup(linea);
        if (!(*palabras)[count])
            error("Error en strdup");
        count++;
    }

    free(linea);
    fclose(f);
    *nPalabras = count;
}

//Devolvera la posición desde donde empieza la palabra
int *buscarPalabra(char **crucigrama, char *palabra, int filasTotal, int columnasTotal, int filaInicial, int filaFinal){

    int *coordenadas = NULL;

    for(int i=filaInicial; i<filaFinal; i++){
        for(int j=0; j<columnasTotal; j++){
            if(palabra[0] == crucigrama[i][j]){
                int palabraEncontrada = 0;
                int posicionPalabra = 1;
                int length = strlen(palabra);
                for(int k = -1; k<=1 && !palabraEncontrada; k++){
                    for(int c = -1; k<=1 && !palabraEncontrada; c++){
                        if(k==0 || c ==0) continue;
                        int diagX = i + k;
                        int diagY = j + c;
                        if((diagX>=0 && diagX < filasTotal) && (diagY>=0 && diagY<columnasTotal)){
                            while(palabra[posicionPalabra]==crucigrama[diagX][diagY]){
                                posicionPalabra++;
                                diagX + k;
                                diagY + c;  
                            }
                            if(posicionPalabra == length - 1){
                                palabraEncontrada = 1;
                                coordenadas = malloc(2*sizeof(int));
                                coordenadas[0] = i;
                                coordenadas[1] = j;
                            }else{
                                posicionPalabra = 0;
                            }
                        }
                    }  
                }
            }
        }
    }

    return coordenadas;

}


void eliminarLinea(char ***lineas, int *numLineas, int index) {
    if (index < 0 || index >= *numLineas) return;

    // Liberar la memoria de la línea
    free((*lineas)[index]);

    // Mover todas las líneas siguientes una posición hacia arriba
    for (int i = index; i < *numLineas - 1; i++) {
        (*lineas)[i] = (*lineas)[i + 1];
    }

    // Reducir el tamaño del arreglo
    char **tmp = realloc(*lineas, (*numLineas - 1) * sizeof(char *));
    if (tmp != NULL || *numLineas - 1 == 0) {
        *lineas = tmp; // reasignar solo si realloc tuvo éxito
    }

    (*numLineas)--;
}

int main(){
    int filas, columnas;
    char **crucigrama;

    int nPalabras;
    char **palabras;

    leerArchivo("crucigrama.txt", &filas, &columnas, &crucigrama, &nPalabras, &palabras);
    imprimirCrucigrama(crucigrama, filas, columnas);

    printf("\nPalabras a buscar:\n");
    for (int i = 0; i < nPalabras; i++) {
        printf("%s\n", palabras[i]);
    }


    pid_t root = getpid();
    pid_t pidHijo;

    int fd[NFD][2];
        for (int i = 0; i < NFD; i++) {
        if (pipe(fd[i]) == -1) {
            error("Error creando el pipe");
        }
    }

    for(int i=0; i<NHIJOS; i++){
        pidHijo = fork();
        if(!pidHijo){
            pidHijo = i;
            break;
        }else if (pidHijo == -1){
            error("Error en la creación del proceso");
        }
        
    }

    if(root == getpid()){
        for(int i=0; i<NFD; i++){
            close(fd[i][1]);
        }

        for(int j=0; j<NFD; j++){
            for(int k=0; k<nPalabras; k++){
                int *posicion = malloc(2*sizeof(int));
                ssize_t lecturaPosicion = read(fd[j][0], posicion, 2*sizeof(int));
                if(lecturaPosicion>0){
                    char palabra[strlen(palabras[k])];
                    ssize_t lecturaPalabra = read(fd[j][0], palabra, strlen(palabra));
                    
                }else{
                    break;
                }
            } 
        }

    }else{
         for(int i=0; i<NFD; i++){
            if(i!=pidHijo) close(fd[i][1]);
            close(fd[i][0]);
        }

        int mitadMatriz = filas / 2;

        int filaInicial = pidHijo*mitadMatriz;
        int filaFinal = (pidHijo==0) ? mitadMatriz: filas;

        for(int i=0; i<nPalabras; i++){
            int *ubicacion = buscarPalabra(crucigrama, palabras[i], filas, columnas, filaInicial, filaFinal);
            if(ubicacion!=NULL){
                ssize_t escrituraUbicaion = write(fd[pidHijo][1], ubicacion, 2*sizeof(int)); 
                if (escrituraUbicaion <= 0) { 
                    error("error al escribir la ubicacion de la palabra"); 
                }
                ssize_t escrituraPalabra = write(fd[pidHijo][1], palabras[i], strlen(palabras[i]));
                 if (escrituraPalabra <= 0) { 
                    error("error al escribir la ubicacion de la palabra"); 
                }

            }
             free(ubicacion);
        }

         close(fd[pidHijo][1]);
       


    }
    

    // Liberar memoria
    for (int i = 0; i < filas; i++)
        free(crucigrama[i]);
    free(crucigrama);

    for (int i = 0; i < nPalabras; i++)
        free(palabras[i]);
    free(palabras);
}
