#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_LINE 80 
#define HISTORY_COUNT 10 /* Esta define el contador del historial */
#define BUFFER_SIZE 50 
#define MAX_HISTORY 50 /* Máximo número de comandos en el historial */

char buffer[BUFFER_SIZE];
char history[MAX_HISTORY][MAX_LINE]; /* Arreglo de almacenamiento del historial */
int history_index = 0; /* Índice de seguimiento del historial */
int total_commands = 0; /* Total de comandos ingresados */

/* Función handle_SIGINT */
void handle_SIGINT() {
    write(STDOUT_FILENO, buffer, strlen(buffer));
    int start = (total_commands < HISTORY_COUNT) ? 0 : total_commands - HISTORY_COUNT;
    for (int i = start; i < total_commands; i++) {
        int index = i % MAX_HISTORY;
        if (strlen(history[index]) > 0) {
            char output[MAX_LINE + 20]; // Aumentamos el tamaño del buffer
            snprintf(output, sizeof(output), "%d %s\n", i + 1, history[index]);
            write(STDOUT_FILENO, output, strlen(output));
        }
    }
    exit(0);
}

/* Función Historial */
void add_history(char inputBuffer[]) {
    strncpy(history[history_index], inputBuffer, MAX_LINE);
    history_index = (history_index + 1) % MAX_HISTORY;
    total_commands++;
}

void print_last_commands() {
    int start = (total_commands < HISTORY_COUNT) ? 0 : total_commands - HISTORY_COUNT;
    for (int i = start; i < total_commands; i++) {
        int index = i % MAX_HISTORY;
        if (strlen(history[index]) > 0) {
            printf("%d %s\n", i + 1, history[index]);
        }
    }
}

void setup(char inputBuffer[], char *args[], int *background) {
    int length, i, start, ct; 
    ct = 0;     
    /* leer lo que el usuario escribe en la línea de comandos */
    length = read(STDIN_FILENO, inputBuffer, MAX_LINE);  

    start = -1;
    if (length == 0)
        exit(0);    /* ^d was entered, end of user command stream */        
    if (length < 0){
        perror("error reading the command");
        exit(-1);    /* terminate with error code of -1 */        
    }
    /* examine every character in the inputBuffer */
    for (i = 0; i < length; i++) { 
        switch (inputBuffer[i]){
            case ' ':
            case '\t' :     /* argument separators */          
                if (start != -1){
                    args[ct] = &inputBuffer[start];   /* set up pointer */ 
                    ct++;
                }
                inputBuffer[i] = '\0'; /* add a null char; make a C string */
                start = -1;
                break;

            case '\n':   /* should be the final char examined */               
                if (start != -1){
                    args[ct] = &inputBuffer[start];     
                    ct++;
                }
                inputBuffer[i] = '\0';
                args[ct] = NULL; /* no more arguments to this command */
                break;

            default :  /* some other character */           
                if (start == -1)
                    start = i;
                if (inputBuffer[i] == '&'){
                    *background  = 1;
                    inputBuffer[i] = '\0';
                }
        } 
    }    
    args[ct] = NULL; /* just in case the input line was > 80 */
} 

// MAIN 
int main(void) {
    char inputBuffer[MAX_LINE]; /* búfer para almacenar los comandos introducidos */
    int background;  /* igual a 1 si el comando termina con '&' */  
    char *args[MAX_LINE / 2 + 1];
    
    // Estructura de controlador de sigacción (señal)
    struct sigaction handler;
    handler.sa_handler = SIG_IGN;
    //handler.sa_handler = handle_SIGINT; 
    sigemptyset(&handler.sa_mask);
    handler.sa_flags = 0;
    sigaction(SIGINT, &handler, NULL);
    strcpy(buffer, "\nSe presionó <ctrl> + <c>\n");

    while (1) {   /* Program terminates normally inside setup */         
        background = 0;
        printf(" ::myshellsmall-> $$ \n");
        setup(inputBuffer, args, &background);   /* get next command */     
        add_history(inputBuffer);
        print_last_commands();

        // Agregamos un proceso hijo con FORK
        pid_t pid = fork(); // Crea un proceso hijo
        if (pid < 0) { // Si fork() devuelve un # < 0 da error
            perror("Fork failed");
            exit(1);
        } else if (pid == 0) { 
            //ignorar el SIGINT en el proceso hijo 
            struct sigaction child_handler; 
            child_handler.sa_handler = SIG_IGN; 
            sigemptyset(&child_handler.sa_mask); 
            child_handler.sa_flags = 0; 
            sigaction(SIGINT, &child_handler, NULL);
            execvp(args[0], args); 
            perror("execvp failed"); 
            exit(1);
        } else { 
            if (background == 0) {
                waitpid(pid, NULL, 0);
            }
        }
    }
}








