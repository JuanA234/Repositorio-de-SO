#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXIMO 1000000
#define MINIMO 1

void append(char **cadena, int *pos, int *capacidad, const char *texto)
{
    int len = strlen(texto);
    if (len + 1 + *pos > *capacidad)
    {
        *capacidad = (*pos + len + 1) * 2;
        *cadena = realloc(*cadena, *capacidad);
        if (*cadena == NULL)
        {
            printf("Error al reservar memoria\n");
            exit(1);
        }
    }

    memcpy(*cadena + *pos, texto, len + 1);
    *pos += len;
}

int main()
{

    FILE *file;
    file = fopen("practica.txt", "r");
    if (!file)
    {
        perror("Error en la lectura de archivo");
        exit(1);
    }

    char mode[20];
    char *cadena = NULL;
    char *cadena_salida = NULL;
    int capacidad_salida = 0;
    int pos_salida = 0;

    fgets(mode, 20, file);
    mode[strcspn(mode, " \n")] = '\0';

    if (strcmp(mode, "ENCODE") == 0)
    {
        cadena = malloc(MAXIMO + 1);

        if (cadena == NULL)
        {
            printf("Error al reservar memoria\n");
            exit(1);
        }
        if (fgets(cadena, MAXIMO + 1, file))
        {
            cadena[strcspn(cadena, "\n")] = '\0';
            int len = strlen(cadena);
            if (len < MINIMO)
            {
                printf("Error, la cadena debe tener al menos %d caracteres\n", MINIMO);
                free(cadena);
                cadena = NULL;
                exit(1);
            }
            else
            {
                int contador = 0;
                char vocalTemp = cadena[0];
                for (int i = 0; cadena[i] != '\0'; i++)
                {
                    if (vocalTemp == cadena[i])
                    {
                        contador++;
                        if (cadena[i + 1] == '\0')
                        {
                            char temp[32];
                            temp[0] = vocalTemp;
                            snprintf(temp + 1, sizeof(temp) - 1, "%d", contador);
                            append(&cadena_salida, &pos_salida, &capacidad_salida, temp);
                        }
                    }
                    else
                    {
                        char temp[32];
                        temp[0] = vocalTemp;
                        snprintf(temp + 1, sizeof(temp) - 1, "%d", contador);
                        append(&cadena_salida, &pos_salida, &capacidad_salida, temp);

                        vocalTemp = cadena[i];
                        contador = 1;

                        if (cadena[i + 1] == '\0')
                        {
                            char temp[32];
                            temp[0] = vocalTemp;
                            snprintf(temp + 1, sizeof(temp) - 1, "%d", contador);
                            append(&cadena_salida, &pos_salida, &capacidad_salida, temp);
                        }
                    }
                }
            }
        }
        free(cadena);
        cadena = NULL;
    }
    else if (strcmp(mode, "DECODE") == 0)
    {
        size_t tam = 0;
        if (getline(&cadena, &tam, file))
        {
            cadena[strcspn(cadena, "\n")] = '\0';
            int suma = 0;
            char vocalTemp[0];
            vocalTemp[0] = cadena[0];
            while (*cadena)
            {
                if (isdigit(*cadena))
                {
                    int num = strtol(cadena, &cadena, 10);
                    suma += num;
                    if (suma >= MAXIMO)
                    {
                        printf("La cadena es muy grande, fijese que la suma de los numeros sea menor de %d \n", MAXIMO);
                        free(cadena);
                        cadena = NULL;
                        exit(1);
                    }
                    for(int i = 1; i <= num; i++){
                        append(&cadena_salida, &pos_salida, &capacidad_salida, vocalTemp);
                    }
                    vocalTemp[0] = *cadena;
                }
                cadena++;
            }
        }
    }
    else
    {
        printf("Error, palabra MODE incorrecta: %s \n", mode);
    }

    fclose(file);
    printf("%s", cadena_salida);
    printf("\n");

    free(cadena_salida);
    cadena_salida = NULL;

    return 0;
}