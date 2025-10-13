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

    pid_t root = getpid();
    int i;

    int fd[2][2];
    for (int i = 0; i < 2; i++) {
        if (pipe(fd[i]) == -1) {
            error("Error creando el pipe");
        }
    }

    for(i=0; i<2; i++){
        pid_t child = fork(); 
        if(!child){
            break;
        }else if(child == -1){
             error("error en la creación del nuevo proceso");
        }
    }

    if(root == getpid()){
        for(int i=0; i<2; i++){
            close(fd[i][1]);
        }

        char buffer[256];

        
        for (int i = 0; i < 2; i++) wait(NULL);

        printf("[%d] Leyendo desde los hijos: \n", getpid());
        ssize_t bytes;
        for(int i=0; i<2; i++){
            while((bytes=read(fd[i][0], buffer, sizeof(buffer)))>0){
                buffer[bytes] = '\0';
                printf("%s", buffer);
            }
            close(fd[i][0]);
        }
      

    }else{
        printf("[%d] Enviando líneas al padre...\n\n", getpid());
        for(int j=0; j<2; j++){
            close(fd[j][0]);
            if(i!=j){
                close(fd[j][1]);
            }
        }
        if(i==0){
            int n=0;
            char **contenido = leerArchivo("file1.txt", &n);
            for(int j=0; j<n; j++){
                write(fd[i][1], contenido[j], strlen(contenido[j]));
                write(fd[i][1], "\n", 1);
            }

           close(fd[i][1]);

           for(int i=0; i<n; i++){
            free(contenido[i]);
           }
           free(contenido);

        }else{
            int n=0;
            char **contenido = leerArchivo("file2.txt", &n);

             for(int j=0; j<n; j++){
                write(fd[i][1], contenido[j], strlen(contenido[j]));
                write(fd[i][1], "\n", 1);
            }

           close(fd[i][1]);

           for(int i=0; i<n; i++){
            free(contenido[i]);
           }
           free(contenido);
        }
    }

    return EXIT_SUCCESS;
}
