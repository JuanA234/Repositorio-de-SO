#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h>

void *oldhandler;
void sighandler(int sig) {
    printf("sig %d capturada\n", sig);
}

int main() {
    pid_t pidhijo;
    oldhandler = signal(SIGUSR1, sighandler);
    if (oldhandler == SIG_ERR) {
        perror("signal:");
        exit(EXIT_FAILURE);
    }

    pidhijo = fork();
    switch (pidhijo) {
        case -1:
            perror("fork");
            exit(EXIT_FAILURE);

        case 0: // Proceso hijo
            pause();
            printf("[%d] Terminando\n", getpid());
            break;

        default: // Proceso padre
            usleep(100);  // Pequeña espera (100 microsegundos)
            kill(pidhijo, SIGUSR1);
            printf("[%d] Señal enviada\n", getpid());
            wait(NULL);
    }

    if(signal(SIGUSR1, oldhandler)==SIG_ERR){
        perror("signal: ");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
