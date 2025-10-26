#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/sem.h>

// Estructura para operaciones de semáforo
struct sembuf operacion;

int main() {
    int shm_id, sem_id;
    int *ptr;
    int shm_size = sizeof(int);

    // Crear segmento de memoria compartida
    shm_id = shmget(IPC_PRIVATE, shm_size, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (shm_id == -1) {
        perror("Error en shmget");
        exit(EXIT_FAILURE);
    }

    // Crear conjunto de 1 semáforo
    sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (sem_id == -1) {
        perror("Error en semget");
        exit(EXIT_FAILURE);
    }

    // Inicializar semáforo en 0 (cerrado)
    if (semctl(sem_id, 0, SETVAL, 0) == -1) {
        perror("Error en semctl SETVAL");
        exit(EXIT_FAILURE);
    }

    // Crear proceso hijo
    if (fork() == 0) {
        // ---------- PROCESO HIJO ----------
        ptr = (int *)shmat(shm_id, 0, 0);

        printf("[%d] Hijo esperando...\n", getpid());

        // ↓ Esperar a que el semáforo se libere (operación P)
        operacion.sem_num = 0;
        operacion.sem_op = -1;  // Espera hasta que el valor > 0
        operacion.sem_flg = 0;
        semop(sem_id, &operacion, 1);

        // Ya puede continuar
        printf("[%d] Hijo lee valor: %d\n", getpid(), *ptr);

        shmdt(ptr);
        exit(EXIT_SUCCESS);
    } else {
        // ---------- PROCESO PADRE ----------
        ptr = (int *)shmat(shm_id, 0, 0);
        *ptr = 0;

        printf("[%d] Padre inicializa valor...\n", getpid());
        sleep(2);

        // Escribir en la memoria compartida
        *ptr = 42;
        printf("[%d] Padre escribe valor: %d\n", getpid(), *ptr);

        // ↑ Liberar al hijo (operación V)
        operacion.sem_num = 0;
        operacion.sem_op = 1;  // Incrementa el semáforo → despierta al hijo
        operacion.sem_flg = 0;
        semop(sem_id, &operacion, 1);

        wait(NULL);

        // Limpieza
        shmdt(ptr);
        shmctl(shm_id, IPC_RMID, NULL);
        semctl(sem_id, 0, IPC_RMID); // Eliminar semáforo
        printf("[%d] Padre termina.\n", getpid());
    }

    return EXIT_SUCCESS;
}
