#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/stat.h>

int main() {
    int shm_id;
    int shm_size = 1024;   // Tamaño del segmento de memoria compartida
    char *ptr;

    // Crear un segmento de memoria compartida privado
    shm_id = shmget(IPC_PRIVATE, shm_size, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (shm_id == -1) {
        perror("Error al crear la memoria compartida");
        return 1;
    }

    // Adjuntar la memoria al proceso
    ptr = (char *)shmat(shm_id, 0, 0);
    if (ptr == (char *)-1) {
        perror("Error al adjuntar la memoria");
        return 1;
    }

    if (fork() == 0) {
        // ----- PROCESO HIJO -----
        sleep(1); // Espera para que el padre escriba primero
        printf("[%d] Lee el hijo: %s\n", getpid(), ptr);

        sprintf(ptr, "bye!");
        shmdt(ptr); // Desvincular memoria
    } else {
        // ----- PROCESO PADRE -----
        sprintf(ptr, "Holaaa Mundo!");
        printf("[%d] Padre escribe: %s\n", getpid(), ptr);

        wait(NULL); // Espera a que termine el hijo
        printf("[%d] Padre lee después: %s\n", getpid(), ptr);

        shmdt(ptr); // Desvincular memoria
        shmctl(shm_id, IPC_RMID, 0); // Eliminar segmento
    }

    return 0;
}
