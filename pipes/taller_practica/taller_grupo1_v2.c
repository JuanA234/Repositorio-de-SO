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

char **leerArchivo(const char *nombreArchivo, int *numLineas) {
    FILE *f = fopen(nombreArchivo, "r");
    if (!f) error("Error al abrir el archivo");

    char **lineas = NULL;
    char *linea = NULL;
    size_t len = 0;
    ssize_t leidos;
    int count = 0;

    while ((leidos = getline(&linea, &len, f)) != -1) {
        if (linea[leidos - 1] == '\n') linea[leidos - 1] = '\0';
        lineas = realloc(lineas, (count + 1) * sizeof(char *));
        if (!lineas) error("Error realloc");
        lineas[count] = strdup(linea);
        if (!lineas[count]) error("Error strdup");
        count++;
    }

    free(linea);
    fclose(f);
    *numLineas = count;
    return lineas;
}

int main() {
    pid_t root = getpid();
    int fd[2][2];

    for (int i = 0; i < 2; i++)
        if (pipe(fd[i]) == -1) error("Error creando pipe");

    int i;
    for (i = 0; i < 2; i++) {
        pid_t child = fork();
        if (child == 0) break; // hijo
        else if (child == -1) error("Error en fork");
    }

    if (root == getpid()) {
        // Proceso padre
        for (int j = 0; j < 2; j++) close(fd[j][1]); // cerrar escritura

        printf("[%d] Leyendo intercaladamente desde los hijos:\n", getpid());

        FILE *pipes[2];
        pipes[0] = fdopen(fd[0][0], "r");
        pipes[1] = fdopen(fd[1][0], "r");

        char *linea1 = NULL, *linea2 = NULL;
        size_t len1 = 0, len2 = 0;
        ssize_t read1, read2;

        while (1) {
            read1 = getline(&linea1, &len1, pipes[0]);
            read2 = getline(&linea2, &len2, pipes[1]);

            if (read1 == -1 && read2 == -1) break;

            if (read1 != -1) {
                linea1[strcspn(linea1, "\n")] = '\0';
                printf("%s (file1)\n", linea1);
            }
            if (read2 != -1) {
                linea2[strcspn(linea2, "\n")] = '\0';
                printf("%s (file2)\n", linea2);
            }
        }

        free(linea1);
        free(linea2);
        fclose(pipes[0]);
        fclose(pipes[1]);

        for (int j = 0; j < 2; j++) wait(NULL);

    } else {
        // Proceso hijo
        for (int j = 0; j < 2; j++) {
            close(fd[j][0]);
            if (i != j) close(fd[j][1]);
        }

        int n = 0;
        char **contenido = (i == 0)
                           ? leerArchivo("file1.txt", &n)
                           : leerArchivo("file2.txt", &n);

        for (int j = 0; j < n; j++) {
            dprintf(fd[i][1], "%s\n", contenido[j]);
            free(contenido[j]);
        }

        free(contenido);
        close(fd[i][1]);
    }

    return EXIT_SUCCESS;
}
