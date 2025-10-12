#include <stdio.h>
#include <stdlib.h>

int main()
{

    int *array;

    // array = malloc(10 * sizeof(int));
    array = calloc(10, sizeof(int));

    
    if (array == NULL)
    {
        printf("Ha ocurrido un error al momento de reservar el espacio de memoria.\n");
        exit(1);
    }

        
       for (int i = 0; i < 10; i++)
    {
        array[i] = i;
    }
    
    printf("Elementos del array: \n");

        for (int i = 0; i < 10; i++)
    {
        printf("%i ", *(array + i)); // == array[i]
    }

    printf("\n");



 

    array = realloc(array,15 * sizeof(int));


    printf("Elementos del array: \n");

    for (int i = 0; i < 15; i++)
    {
        printf("%i ", *(array + i)); // == array[i]
    }

    printf("\n");

    free(array);
    array = NULL;

    return 0;
}