#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h> 
#include <string.h>
#include <ctype.h>

const char alfabeto[26] = {'a', 'b' ,'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};

    
int contador=0;


void signalHandler(int sig){
    if(sig == SIGUSR1){
        contador++;
    }else if(sig == SIGUSR2){
        //printf("Hijo [%d] ", getpid());
        if (contador > 0 && contador <= 26) {
            char letra = 'a' + contador - 1;
            printf("%c", letra);
            fflush(stdout);
        }
        contador = 0;
    }else if(sig == SIGTERM){
        printf("\n");
        fflush(stdout); 
    }

}



void error(char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

char **leerArchivo(const char *nombreArchivo, int *numLineas){

    FILE *f = fopen(nombreArchivo, "r");

    if(!f){
        error("Error al abrir el archivo");
    }   

    char **lineas = NULL;  // Arreglo dinámico de punteros a char
    char *linea = NULL; //Buffer temporal para cada línea
    size_t len = 0; // tamaño del buffer
    ssize_t leidos; // Bytes leídos en cada línea
    int count = 0; //Contador de líneas

    while((leidos = getline(&linea, &len, f))!=-1){
        if(linea[leidos-1]=='\n'){
            linea[leidos-1] = '\0';
        }

        // Aumentar el tamaño del arreglo dinámicamente
        lineas = realloc(lineas, (count + 1) * sizeof(char *));
        if(!lineas){
            error("Error al asignar memoria con realloc");
        }

        // Copiar la línea en memoria nueva
        lineas[count] = strdup(linea);
        if (!lineas[count]) {
            error("Error en strdup");
        }

        count++;

    }

    free(linea);
    fclose(f);

    *numLineas = count;
    return lineas;
}

int main(){

    signal(SIGUSR1, signalHandler);
    signal(SIGUSR2, signalHandler);
    signal(SIGTERM, signalHandler);

    int root = getpid();
    
    pid_t pidHijo[2];

    for(int i=0; i<2; i++){
        pidHijo[i] = fork();
        if(!pidHijo[i]){
            break;
        }else if(pidHijo[i]==-1){
            error("Error al crear el proceso nuevo");
        }
    }

    if(root == getpid()){
        int numLineas = 0;
        char **instrucciones = leerArchivo("matriz.txt", &numLineas);

        if (instrucciones == NULL){
            printf("La matriz está vacía.\n");
        }

        for (int i = 0; i < numLineas; i++){
            pid_t destino = (i % 2 == 0) ? pidHijo[0] : pidHijo[1];

            for(int j=0; instrucciones[i][j]!='\n' && instrucciones[i][j]!='\0'; j++){
                char c = tolower(instrucciones[i][j]);
                int pos = -1;
                for(int k=0; k<26; k++){
                    if(c == alfabeto[k]){
                        pos = k+1;
                        break;
                    }     
                }

                if(pos>0){
                    for (int t = 0; t < pos; t++) {
                        kill(destino, SIGUSR1);
                        usleep(15000);
                    }
                    kill(destino, SIGUSR2);
                    usleep(20000);
                }
            }
            // Fin de línea → salto
            kill(destino, SIGTERM);
            usleep(30000);
        }

        for (int i = 0; i < 2; i++) {
            kill(pidHijo[i], SIGKILL);
             wait(NULL);
        }

    }else{
        while(1){
            pause();       
        }
        
        
    }


    return EXIT_SUCCESS;
}