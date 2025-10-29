#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <ctype.h>
#include <regex.h>

#define NHIJOS 3
#define NFD 4
#define TAM_BUFFER 256

const char *patron = "^TRX:[a-zA-Z0-9_]+ -> [a-zA-Z0-9_]+: ([0-9]+(\\.[0-9]+)?)$";
const char *palabrasSospechosas = "\\b(hacker|cashout|vault)\\b";  


void error(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

char **leerArchivo(const char *nombreArchivo, int *numLineas){

    FILE *f = fopen(nombreArchivo, "r");

    if(!f){
        error("Error al abrir el archivo");
    }   

    int cantidad;
    if (fscanf(f, "%d\n", &cantidad) != 1) {
        fclose(f);
        error("Error al leer la cantidad de líneas del archivo");
    }

    char **lineas = malloc(cantidad * sizeof(char *));// Arreglo dinámico de punteros a char
    if (!lineas) {
        fclose(f);
        error("Error al asignar memoria para las líneas");
    }

    char *linea = NULL; //Buffer temporal para cada línea
    size_t len = 0; // tamaño del buffer
    ssize_t leidos; // Bytes leídos en cada línea
    int count = 0; //Contador de líneas

    while( count<cantidad && (leidos = getline(&linea, &len, f))!=-1){
        if(linea[leidos-1]=='\n'){
            linea[leidos-1] = '\0';
        }

        // Copiar la línea al arreglo
        lineas[count] = strdup(linea);
        if (!lineas[count]) {
            error("Error en strdup");
        }
        count++;
    }

    free(linea);
    fclose(f);

    *numLineas = count; // cantidad real leída (por si hay menos)
    return lineas;
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


void filtradoBasico(char ***lineas, int *numLineas){

    regex_t regex;


    if (regcomp(&regex, patron, REG_EXTENDED | REG_NOSUB) != 0) {
            error("No se pudo compilar la expresión regular");
        }

        for(int i=0; i<*numLineas; i++){
            if(regexec(&regex, (*lineas)[i], 0, NULL, 0) != 0){
                eliminarLinea(lineas, numLineas, i);
                i--; 
            }
        }

    regfree(&regex);
}

void verificacionDeConsistencia(char ***lineas, int *numLineas){

    regex_t regex;
    regmatch_t matches[3];

    if (regcomp(&regex, patron, REG_EXTENDED) != 0) {
        error("No se pudo compilar la expresión regular");
    }

    for(int i=0; i<*numLineas; i++){
        if (regexec(&regex, (*lineas)[i], 3, matches, 0) == 0) {
            // matches[1] contiene los índices del número capturado
            char numero_str[50];
            int inicio = matches[1].rm_so;
            int fin = matches[1].rm_eo;

            strncpy(numero_str, &((*lineas)[i][inicio]), fin-inicio);
            numero_str[fin - inicio] = '\0';
            float valor = strtof(numero_str, NULL);

            if(valor<=0 || valor>10000){
                eliminarLinea(lineas, numLineas, i);
                i--;
            }

        }      
    }

    regfree(&regex);
}

void deteccionDeFraude(char ***lineas, int *numLineas){

    regex_t regex;

    if (regcomp(&regex, palabrasSospechosas,REG_EXTENDED | REG_ICASE) != 0) {
        error("No se pudo compilar la expresión regular");
    }

    for(int i=0; i<*numLineas; i++){
        if(regexec(&regex, (*lineas)[i], 0, NULL, 0) == 0){
                eliminarLinea(lineas, numLineas, i);
                i--; 
            }
    }

    regfree(&regex);
}


int main(){


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

        for(int i = 0; i<NFD; i++){
            if(i!=NFD-1){
                close(fd[i][0]);
            }if(i!=0){
                close(fd[i][1]);
            }   
        }

        int numLineas = 0;
        char **contenido = leerArchivo("transacciones.txt", &numLineas);

        if(write(fd[0][1], &numLineas, sizeof(int)) != sizeof(int))
            error("Error con el hijo %d, al escribir la cantidad de lineas \n");

        for(int j=0; j<numLineas; j++){
            //printf("%s \n", contenido[j]);
            write(fd[0][1], contenido[j], strlen(contenido[j]));
            write(fd[0][1], "\n", 1);
        }

        close(fd[0][1]);

        for(int i=0; i<numLineas; i++){
            free(contenido[i]);
        }
        free(contenido);

        for(int k=0; k<NHIJOS; k++) wait(NULL);

        printf("\n[TRANSACCIONES VALIDAS] \n");
        ssize_t brNumLineas = read(fd[NFD-1][0], &numLineas, sizeof(int));
        if (brNumLineas != sizeof(numLineas)) {
            error("Error en la lectura de el numero de lineas en el primer hijo");
        }

        char buffer[TAM_BUFFER];
        char linea[TAM_BUFFER];
        int pos = 0;
        int count = 0;
        char **lineas = malloc(numLineas * sizeof(char*));
        if(!lineas){
            error("Error al asignar memoria en el primer hijo");
        }

        ssize_t lectura;
        while((lectura = read(fd[NFD-1][0], buffer, TAM_BUFFER))>0){
            buffer[lectura] = '\0';
            //printf("%s\n", buffer);
            for(ssize_t i = 0; i<lectura; i++){
                if(buffer[i] == '\n'){
                    linea[pos] = '\0';
                    
                    lineas[count] = strdup(linea);
                    if (!lineas[count]) {
                        error("Error en strdup");
                    }
                    count++;
                    pos = 0; // reiniciar para la siguiente línea

                }else{
                    if(pos<TAM_BUFFER){
                        linea[pos++] = buffer[i];
                }
                }
            }
        }
        close(fd[NFD-1][0]);

        for(int j=0; j<numLineas; j++){
            printf("%s \n", lineas[j]);
        }

        for(int i=0; i<numLineas; i++){
            free(lineas[i]);
        }
        free(lineas);

    }else{

        int readIndex = pidHijo;
        int writeIndex = pidHijo + 1;

        

        for(int i=0; i<NFD; i++){
            if(i!=readIndex){
                close(fd[i][0]);
            }if(i!=writeIndex){
                close(fd[i][1]);
            }
        }

        //Leer el numero de lineas recibido por el padre
        int numLineas;
        ssize_t brNumLineas = read(fd[readIndex][0], &numLineas, sizeof(int));
        if (brNumLineas != sizeof(numLineas)) {
            error("Error en la lectura de el numero de lineas en el primer hijo");
        }

        char buffer[TAM_BUFFER];
        char linea[TAM_BUFFER];
        int pos = 0;
        int count = 0;
        char **lineas = malloc(numLineas * sizeof(char*));
        if(!lineas){
            error("Error al asignar memoria en el primer hijo");
        }
        ssize_t lectura;
        while((lectura = read(fd[readIndex][0], buffer, TAM_BUFFER))>0){
            buffer[lectura] = '\0';
            //printf("%s\n", buffer);
            for(ssize_t i = 0; i<lectura; i++){
                if(buffer[i] == '\n'){
                    linea[pos] = '\0';
                    
                    lineas[count] = strdup(linea);
                    if (!lineas[count]) {
                        error("Error en strdup");
                    }
                    count++;
                    pos = 0; // reiniciar para la siguiente línea

                }else{
                    if(pos<TAM_BUFFER){
                        linea[pos++] = buffer[i];
                }
                }
            }
        }

        close(fd[readIndex][0]);

        

        if(pidHijo==0){
            filtradoBasico(&lineas, &numLineas);
        }else if(pidHijo==1){
            verificacionDeConsistencia(&lineas, &numLineas);;
        }else{
            deteccionDeFraude(&lineas, &numLineas);
        } 

        printf("\n");

        for(int j=0; j<numLineas; j++){
            printf("%s \n", lineas[j]);
        }

        if(write(fd[writeIndex][1], &numLineas, sizeof(int)) != sizeof(int))
            error("Error con el hijo %d, al escribir la cantidad de lineas \n");

        for(int j=0; j<numLineas; j++){
            //printf("%s \n", contenido[j]);
            write(fd[writeIndex][1], lineas[j], strlen(lineas[j]));
            write(fd[writeIndex][1], "\n", 1);
        }

        close(fd[writeIndex][1]);
        for(int i=0; i<numLineas; i++){
            free(lineas[i]);
        }
        free(lineas);

        
        exit(EXIT_SUCCESS);
    }
    return EXIT_SUCCESS;
}
