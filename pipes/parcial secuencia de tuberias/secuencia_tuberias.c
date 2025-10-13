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

int main(){

    return EXIT_SUCCESS;
}