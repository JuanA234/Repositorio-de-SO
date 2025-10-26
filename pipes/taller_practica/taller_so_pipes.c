#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>


void error(char *msg)
{
    perror(msg);
    exit(-1);
}

int leerNumeros(char *filename, int **vec)
{
    int c, numero, totalNumeros;
    FILE *infile;
    infile = fopen(filename, "r");
    if (!infile)
    {
        error("Error fopen\n");
    }
    fscanf(infile, "%d", &totalNumeros);
    *vec = (int *)calloc(totalNumeros, sizeof(int));
    if (!*vec)
    {
        error("error calloc");
    }
    for (c = 0; c < totalNumeros; c++)
    {
        fscanf(infile, "%d", &numero);
        (*vec)[c] = numero;
    }
    fclose(infile);
    return c;
}

long long cantidadParNumeros(int *vector, int cantidadNumeros){
    long long count = 0;
    for(int i=0; i<cantidadNumeros; i++){
        if(vector[i] % 2 == 0){
            count++;
        }
    }
    return count;

}

long long cantidadImparNumeros(int *vector, int cantidadNumeros){
    long long count = 0;
    for(int i=0; i<cantidadNumeros; i++){
        if(vector[i] % 2 == 1){
            count++;
        }
    }
    return count;
}

long long promedioNumeros(int *vector, long long cantidadNumeros){
    long long total = 0;
    for(int i=0; i<cantidadNumeros; i++){
      total += vector[i];
    }

    long long promedio = (total / cantidadNumeros);
    return promedio;

}


int main(){


    /*Las primeras 3 tuberias seran usadas para comunicar los datos del padre a los hijos
    las otras 3 para comunicar los resultados con los procesos hijos
    */
    
    const int TAMANIOFD = 6;
    int fd[TAMANIOFD][2];
        for (int i = 0; i < TAMANIOFD; i++) {
        if (pipe(fd[i]) == -1) {
            error("Error creando el pipe");
        }
    }

    pid_t root = getpid();
    int hijo;
    for(hijo=0; hijo<3; hijo++){
        pid_t child = fork(); 
            if(!child){
                break;
            }else if(child == -1){
                error("error en la creación del nuevo proceso");
            }

    }



    if(root == getpid()){

        int *vector;
        long long cantidadNumeros = leerNumeros("datos.txt", &vector);


        //Cerra extremos innecesarios
        for(int i=0; i<TAMANIOFD; i++){
            if(i<3){//Cierra el extremo de lecturos de las primeras 3 tuberias
                close(fd[i][0]);
            }else if(i>=3){ //cierro el extremo de escritura de las ultimas tres tuberias
                close(fd[i][1]);
            }
        }
  
        
        for(int i=0; i<3; i++){

            int tarea = i;

            // escribir tarea
            if (write(fd[i][1], &tarea, sizeof(tarea)) != sizeof(tarea)) {
                fprintf(stderr, "Padre: error al escribir tarea al hijo %d\n", i);
                exit(EXIT_FAILURE);
            }
            // escribir cantidadNumeros
            if (write(fd[i][1], &cantidadNumeros, sizeof(cantidadNumeros)) != sizeof(cantidadNumeros)) {
                fprintf(stderr, "Padre: error al escribir cantidad al hijo %d\n", i);
                exit(EXIT_FAILURE);
            }

            ssize_t bytesParaEscribir = (ssize_t) cantidadNumeros * sizeof(int);
            ssize_t bytesEscritos = 0;
            char *buffer = (char*)vector;

            while (bytesEscritos < bytesParaEscribir){
                ssize_t escritura = write(fd[i][1], buffer + bytesEscritos, bytesParaEscribir - bytesEscritos);
                if (escritura <= 0) { 
                    error("error al escribir numeros"); 
                }
                bytesEscritos += escritura;
            }
            close(fd[i][1]);
            }
        free(vector);


        for (int i = 0; i < 3; i++) {
            wait(NULL);
        }
   
        //printf("%lld\n", cantidadNumeros);
        printf("Proceso padre \n");

        // leer resultados de los hijos desde fd[3], fd[4], fd[5]
        for(int i=3; i<6; i++){
            long long resultados = 0;
            ssize_t br = read(fd[i][0], &resultados, sizeof(resultados));
            if (br != sizeof(resultados)) {
                fprintf(stderr, "Padre: error leyendo resultado de tuberia %d (leidos=%zd)\n", i, br);
            } else {
                printf("El hijo %d envio el resultado %lld\n", i - 3, resultados);
            }
            close(fd[i][0]);
        }
   

    }else{
        printf("Proceso hijo #%d \n\n", hijo);

        int readIndex = hijo;        // pipe para leer: fd[readIndex][0]
        int writeIndex = hijo + 3;   // pipe para escribir resultado: fd[writeIndex][1]

        //Cerra extremos innecesarios
        for(int i=0; i<TAMANIOFD; i++){
            if(i<3){//Cierra el extremo de escritura de las primeras 3 tuberias
                close(fd[i][1]);
            }else if(i>=3){ //cierro el extremo de lectura de las ultimas tres tuberias
                close(fd[i][0]);
            }
        }
        
        // leer tarea
        int tarea;
        ssize_t br_tarea = read(fd[readIndex][0], &tarea, sizeof(tarea));
        if (br_tarea != sizeof(tarea)) {
            fprintf(stderr, "Hijo %d: error leyendo tarea (leidos=%zd)\n", hijo, br_tarea);
            exit(EXIT_FAILURE);
        }


        long long cantidadNumeros;
        ssize_t br_cant = read(fd[readIndex][0], &cantidadNumeros, sizeof(cantidadNumeros));
        if (br_cant != sizeof(cantidadNumeros)) {
            fprintf(stderr, "Hijo %d: error leyendo cantidad (leidos=%zd)\n", hijo, br_cant);
            exit(EXIT_FAILURE);
        }

        if (cantidadNumeros < 0) {
            fprintf(stderr, "Hijo %d: cantidad de numeros negativa\n", hijo);
            exit(EXIT_FAILURE);
        }

        int *vec = malloc((size_t)cantidadNumeros * sizeof(int));
        if (!vec) error("malloc");

        ssize_t bytesParaLeer = (ssize_t)cantidadNumeros * (ssize_t)sizeof(int);
        ssize_t bytesLeidos = 0;
        char *buf = (char*)vec;
        while (bytesLeidos < bytesParaLeer) {
            ssize_t r = read(fd[readIndex][0], buf + bytesLeidos, bytesParaLeer - bytesLeidos);
            if (r < 0) error("read");
            if (r == 0) break; // EOF inesperado
            bytesLeidos += r;
        }

        if (bytesLeidos != bytesParaLeer) {
            fprintf(stderr, "Hijo %d: no se leyeron todos los bytes esperados (%zd de %zd)\n",
                    hijo, bytesLeidos, bytesParaLeer);
            free(vec);
            exit(EXIT_FAILURE);
        }

             // calcular según tarea (tarea == hijo, por convención)
        long long resultado = 0;
        if (tarea == 0 && hijo == 0) {
            resultado = cantidadParNumeros(vec, cantidadNumeros);
        } else if (tarea == 1 && hijo == 1) {
            resultado = cantidadImparNumeros(vec, cantidadNumeros);
        } else if (tarea == 2 && hijo == 2) {
            resultado = promedioNumeros(vec, cantidadNumeros);
        } else {
            // Si no coincide la tarea, devolver 0 o un código de error
            resultado = 0;
        }

        ssize_t bytesParaEscribir = sizeof(resultado);
        ssize_t bytesEscritos = 0;
        char *wb = (char*)&resultado;
        while (bytesEscritos < bytesParaEscribir) {
            ssize_t w = write(fd[writeIndex][1], wb + bytesEscritos, bytesParaEscribir - bytesEscritos);
            if (w <= 0) error("write hijo");
            bytesEscritos += w;
        }

        close(fd[writeIndex][1]);
        free(vec);
       
    }
    


    return EXIT_SUCCESS;
}