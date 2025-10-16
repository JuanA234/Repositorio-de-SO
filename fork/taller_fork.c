#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

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
        printf("%d\n", numero);
    }
    fclose(infile);
    return c;
}

long long leerTotal(int n)
{
    FILE *infile;
    long long sumap = 0, total = 0;
    infile = fopen("out.txt", "r");
    if (!infile)
        error("Error padre archivo resultados");
    for(int i=0; i<n; i++){
        fscanf(infile, "%lld", &sumap);
        total += sumap;
    }
    return total;
}

int main()
{

    int n;

    printf("Ingresar número de hijos: ");
    scanf("%d", &n);


    //int limites[n][n];
    int *vector;
    long long cantidadNumeros = leerNumeros("test2.in", &vector);
    long long suma = 0;



    remove("out.txt");
    FILE *archivo = fopen("out.txt", "a");

    if(archivo==NULL){
        error("No se pudo abrir el archivo");
    }

    int delta = cantidadNumeros/n;
    int resto = cantidadNumeros % n; //Cuantos elementos sobran
  

    int i = 0;
    pid_t root = getpid();

    for(i = 0; i < n; i++){
        pid_t pid = fork();
  
        if(pid == 0){
            break;
        }else if(pid == -1){
            error("Error en la creación del nuevo proceso");
        }
    }

    if(root == getpid())
    { // Si es el proceso padre
        for (int j = 0; j < n; j++)
        {
            wait(NULL); // Espera a que los procesos hijos terminen   
        }
        printf("Resultado final: %lld \n", leerTotal(n)); //Imprimir el resultado final
        printf("Finalizando padre %d\n", getpid()); // Imprime un mensaje de finalización del proceso padre
    }else{
        int limiteInicial = delta*i;
        int limiteFinal = delta*(i+1);
   
            for(int j = limiteInicial; j<limiteFinal; j++){
                    suma += vector[j];
             
            }

            if(resto != 0 && i == n-1){
                for(int k=0; k<resto; k++){
                        suma += vector[limiteFinal+k];
                }
            }

            printf("El proceso hijo %d sumo %lld, ppid:%d \n", getpid(), suma, getppid());
            fprintf(archivo, "%lld\n", suma);
}
    

    /*
    if(i==0){
        for(int i=0; i<delta; i++){
            suma += vector[i];
        }

        printf("El proceso hijo %d sumo %d \n", getpid(), suma);
        fprintf(archivo, "%d\n", suma);

    }else if(i==1){
    for(int i=delta; i<cantidadNumeros; i++){
            suma+= vector[i];

        }
       
        printf("El proceso hijo %d sumo %d \n", getpid(), suma);
        fprintf(archivo, "%d\n", suma);
    }


   
    if(i==2){
         wait(NULL);
        printf("Resultado final: %d \n", leerTotal());
   
    }
    */

    
       


    return 0;
}
