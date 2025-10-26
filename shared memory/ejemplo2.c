#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <sys/shm.h>
#include <sys/stat.h>

#define MAX_SIZE 10000000  // 10 millones de elementos

int main() {
    double *a, *b, *result;
    int i;
    int shm_idA, shm_idB, shm_idR;
    int shm_size = MAX_SIZE * sizeof(double);

    // Crear tres segmentos de memoria compartida
    shm_idA = shmget(IPC_PRIVATE, shm_size, IPC_CREAT | S_IRUSR | S_IWUSR);
    shm_idB = shmget(IPC_PRIVATE, shm_size, IPC_CREAT | S_IRUSR | S_IWUSR);
    shm_idR = shmget(IPC_PRIVATE, sizeof(double), IPC_CREAT | S_IRUSR | S_IWUSR);

    // Asociar los segmentos de memoria al espacio del proceso
    a = shmat(shm_idA, 0, 0);
    b = shmat(shm_idB, 0, 0);
    result = shmat(shm_idR, 0, 0);

    // Inicializar los arreglos a y b
    for (i = 0; i < MAX_SIZE; i++) {
        a[i] = i;
        b[i] = i - 0.5;
    }

    // Crear proceso hijo
    if (!fork()) {
        // --- PROCESO HIJO ---
        double temp = 0.0;

        // Calcular el producto punto (dot product)
        for (i = 0; i < MAX_SIZE; i++) {
            temp += a[i] * b[i];
        }

        // Guardar el resultado en memoria compartida
        *result = temp;

        printf("[%d] Resultado parcial del hijo: %f\n", getpid(), temp);

        // Desasociar la memoria compartida
        shmdt(a);
        shmdt(b);
        shmdt(result);
    } else {
        // --- PROCESO PADRE ---
        wait(NULL);  // Esperar a que el hijo termine

        // Leer el resultado del hijo desde memoria compartida
        printf("[%d] Resultado final del padre: %f\n", getpid(), *result);

        // Desasociar los segmentos
        shmdt(a);
        shmdt(b);
        shmdt(result);

        // Eliminar los segmentos de memoria
        shmctl(shm_idA, IPC_RMID, 0);
        shmctl(shm_idB, IPC_RMID, 0);
        shmctl(shm_idR, IPC_RMID, 0);
    }

    return 0;
}
