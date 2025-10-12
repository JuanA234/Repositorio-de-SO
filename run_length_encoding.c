#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LIMITE 1000000  
#define MINIMO 1

char *cadena = NULL;
int capacidad = 0;
int pos = 0;

void append(char **cadena, int *pos, int *capacidad, const char *texto) {
    int len = strlen(texto);
    if (*pos + len + 1 > *capacidad) {
        *capacidad = (*pos + len + 1) * 2;
        *cadena = realloc(*cadena, *capacidad);
        if (*cadena == NULL) { fprintf(stderr, "OOM\n"); exit(1); }
    }
    memcpy(*cadena + *pos, texto, len + 1);
    *pos += len;
}

int main(void) {
    // reservar buffer para fgets
    char *entrada = malloc(LIMITE + 1);
    if (!entrada) { fprintf(stderr, "OOM\n"); return 1; }

    printf("Proporciona una cadena: ");
    if (!fgets(entrada, LIMITE + 1, stdin)) {
        fprintf(stderr, "Error de lectura\n");
        free(entrada);
        return 1;
    }

    entrada[strcspn(entrada, "\n")] = '\0';

    if ((int)strlen(entrada) < MINIMO) {
        printf("Error: debes agregar minimo %d caracteres\n", MINIMO);
        free(entrada);
        free(cadena);
        return 0;
    }

    int contador = 0;
    char vocalTemp = entrada[0];

    for (int i = 0; entrada[i] != '\0'; i++) {
        if (entrada[i] == vocalTemp) {
            contador++;
            if (entrada[i + 1] == '\0') {
                char tmp[32];
                tmp[0] = vocalTemp;
                snprintf(tmp + 1, sizeof(tmp) - 1, "%d", contador);
                append(&cadena, &pos, &capacidad, tmp);
            }
        } else {
            char tmp[32];
            tmp[0] = vocalTemp;
            snprintf(tmp + 1, sizeof(tmp) - 1, "%d", contador);
            append(&cadena, &pos, &capacidad, tmp);

            vocalTemp = entrada[i];
            contador = 1;

                if (entrada[i + 1] == '\0') {
                char tmp[32];
                tmp[0] = vocalTemp;
                snprintf(tmp + 1, sizeof(tmp) - 1, "%d", contador);
                append(&cadena, &pos, &capacidad, tmp);
            }
        }
    }

    printf("\nSalida: %s\n", cadena);

    free(entrada);
    free(cadena);
    return 0;
}
