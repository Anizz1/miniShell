#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 50

static char buffer[BUFFER_SIZE];

/* funci�n de tratamiento de se�al */
void handle_SIGINT() {
	write(STDOUT_FILENO,buffer,strlen(buffer));

	exit(0);
}

int main(int argc, char *argv[]) 
{
	/* configura el descriptor de la se�al */
	struct sigaction handler;
	handler.sa_handler = handle_SIGINT; 
	sigaction(SIGINT, &handler, NULL);

	//gnera el mensaje de salida
	strcpy(buffer,"Captura de <ctrl><c>\n");

	/* Espera hasta que se pulse <control> <C> */
	while (1)
		;

	return 0;
}

