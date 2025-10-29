#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <ctype.h>
#include <regex.h>

#define NHIJOS 2
#define NFD 5
#define TAM_BUFFER 256

const char *patron = "^REQ:\\s*([^;]+);\\s*(.+)$";
const char *palabrasCriticas = "\\b(servidor|bloqueo|caída)\\b";


void error(char *msg){
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


int esUrgente(char *linea){
      regex_t regex;

    if (regcomp(&regex, "URGENTE" ,REG_EXTENDED | REG_ICASE) != 0) {
        error("No se pudo compilar la expresión regular en la linea 70");
    }

    if(regexec(&regex, linea, 0, NULL,0)==0){
        return 1;
    }
     regfree(&regex);
     return 0;

}

int validarFormato(char *linea){
    regex_t regex;

    if (regcomp(&regex, patron, REG_EXTENDED | REG_NOSUB) != 0) {
            error("No se pudo compilar la expresión regular");
    }

    if(regexec(&regex, linea, 0, NULL,0)==0){
        return 1;
    }

    regfree(&regex);
    return 0;

}

int esCritico(char *linea){

    regex_t regex;


    if (regcomp(&regex, palabrasCriticas ,REG_EXTENDED | REG_ICASE) != 0) {
        error("No se pudo compilar la expresión regular en la linea 70");
    }

    if(regexec(&regex, linea, 0, NULL,0)==0){
        return 1;
    }
     regfree(&regex);
     return 0;
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
        

    if(root==getpid()){

        for(int i = 0; i<NFD; i++){
            if(i!=2 && i!=3 && i!=4){
                close(fd[i][0]);
            }
            if(i!=0){
                close(fd[i][1]);
            }   
        }

        int numLineas = 0;
        char **contenido = leerArchivo("solicitudes.txt", &numLineas);

        if(write(fd[0][1], &numLineas, sizeof(int)) != sizeof(int))
            error("Error con el padre, al escribir la cantidad de lineas \n");

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

        for(int i=4 ; i>1; i--){
            char *mensajePrioridad = (i == 2) ? "[BAJA PRIORIDAD]" :
                                    (i == 3)  ? "[URGENTES]" :
                                    (i == 4)  ? "[CRITICAS]" :
                                                    "";

        

            printf("\n%s \n", mensajePrioridad);


            char buffer[TAM_BUFFER];
            char linea[TAM_BUFFER];
            int pos = 0;
            int count = 0;

            ssize_t lectura;
            while((lectura = read(fd[i][0], buffer, TAM_BUFFER))>0){
                buffer[lectura] = '\0';
                //printf("%s\n", buffer);
                for(ssize_t j = 0; j<lectura; j++){
                    if(buffer[j] == '\n'){
                        linea[pos] = '\0';
                        printf("%s\n", linea);
                        count++;
                        pos = 0; // reiniciar para la siguiente línea

                    }else{
                        if(pos<TAM_BUFFER){
                            linea[pos++] = buffer[j];
                    }
                    }
                }
            }

            close(fd[i][0]);
        }



    }else{

        int readIndex = pidHijo;

        if(pidHijo == 0){
            for(int i=0; i<NFD; i++){
                if(i!=readIndex){
                    close(fd[i][0]);
                }if(i!=1 && i!=2){
                    close(fd[i][1]);
                }
            }
        }else if(pidHijo==1){
            for(int i=0; i<NFD; i++){
                if(i!=readIndex){
                    close(fd[i][0]);
                }if(i!=3 && i!=4){
                    close(fd[i][1]);
                }
            }
        }


            //Leer el numero de lineas recibido por el padre
            int numLineas;
            ssize_t brNumLineas = read(fd[readIndex][0], &numLineas, sizeof(int));
            if (brNumLineas != sizeof(numLineas)) {
                error("Error en la lectura de el numero de lineas");
            }

            char buffer[TAM_BUFFER];
            char linea[TAM_BUFFER];
            int pos = 0;
            int count = 0;
            char **lineas = malloc(numLineas * sizeof(char*));
            if(!lineas){
                error("Error al asignar memoria ");
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

            printf("\n");
        

            char **bufferEscritura = malloc(numLineas * sizeof(char*));
            if(pidHijo==0){
                int contador = 0;
               for(int j=0; j<numLineas; j++){
                if(!validarFormato(lineas[j]) || !esUrgente(lineas[j])){   
                    write(fd[2][1], lineas[j], strlen(lineas[j]));
                    write(fd[2][1], "\n", 1);
                }else{
                    lineas[j][ strlen(lineas[j]) + 1] = '\0';
                    bufferEscritura[contador] = strdup(lineas[j]);
                    contador++;
                }
               }
               close(fd[2][1]);
               if(write(fd[1][1], &contador, sizeof(int)) != sizeof(int))
                    error("Error al escribir la cantidad de lineas en la linea 257 \n");
                for(int k=0; k<contador; k++){
                    //printf("%s \n", contenido[j]);
                    write(fd[1][1], bufferEscritura[k], strlen(bufferEscritura[k]));
                    write(fd[1][1], "\n", 1);
                }
                close(fd[1][1]);
            }else if(pidHijo==1){
               for(int j=0; j<numLineas; j++){
                if(!esCritico(lineas[j])){   
                    write(fd[3][1], lineas[j], strlen(lineas[j]));
                    write(fd[3][1], "\n", 1);
                }else{
                    write(fd[4][1], lineas[j], strlen(lineas[j]));
                    write(fd[4][1], "\n", 1);
                }
               }
               close(fd[3][1]);
               close(fd[4][1]);
        
            }  
    }

    return EXIT_SUCCESS;
}