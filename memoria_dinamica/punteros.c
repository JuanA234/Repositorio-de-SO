#include<stdio.h>

int main(){

    int numero = 5;
    int *puntero = &numero;

    printf("\nDirección de memoria de la variable número: %p\n", &numero);
    printf("Dirección de memoria a la que apunta el puntero: %p\n\n", puntero);

    printf("Valor original de la variable numero: %d\n", numero);
    printf("Valor al que apunta el puntero: %d\n", *puntero);
    printf("\n\n");

    *puntero = 15;

    printf("Valor actualizado al que apunta el puntero: %d\n", *puntero);
    printf("Valor de la variable número: %d\n", numero);

    return 0;
}