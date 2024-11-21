#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include "fav_commands.h"

char* fav_list[100];
char* fav_file_route;

int parse_command(char* command,  char** args){

    int i = 0;
    char *token = strtok(command, " \t\r\n");

    while (token != NULL){
        args[i++] = token;
        token = strtok(NULL, " \t\r\n");
    }
    args[i] = NULL;

    return i;
}

int parse_pipe(char* command, char** pipeArgs){
    int i = 0;
    char *token = strtok(command, "|");

    while (token != NULL){
        pipeArgs[i++] = token;
        token = strtok(NULL,"|");
    }

    pipeArgs[i] = NULL;

    return i;
}

int parse_and(char* command, char** AND_ARRAY){
    int i = 0;
    char* token = strtok(command, "&&");

    while (token != NULL){
        AND_ARRAY[i++] = token;
        token = strtok(NULL,"&&");
    }

    AND_ARRAY [i] = NULL;

    return i;
}

void exec_(char** pipeArgs, int numPipes) {

    if (numPipes == 1) {
        char** args = malloc(100 * sizeof(char*));
        parse_command(pipeArgs[0], args);
        
        if (execvp(args[0], args) == -1)
            printf("error\n");

    } 
    else {
        int (*p)[2] = malloc((numPipes - 1) * sizeof(int[2]));

        for (int i = 0; i < numPipes; i++) {
            pipe(p[i]);

            if (fork() == 0) { 
                if (i > 0) {
                    
                    close(p[i-1][1]); 
                    dup2(p[i-1][0], STDIN_FILENO); 
                    close(p[i-1][0]); 
                }

                if (i < numPipes - 1) {
                    
                    close(p[i][0]);  
                    dup2(p[i][1], STDOUT_FILENO); 
                    close(p[i][1]);  
                }

                char** args = malloc(100 * sizeof(char*));
                parse_command(pipeArgs[i], args);

                if (execvp(args[0], args) == -1)
                    printf("Error %d\n", i + 1);

                free(args);
                exit(EXIT_FAILURE); 
            } else {

                if (i > 0) {
                    close(p[i-1][0]);
                    close(p[i-1][1]);
                }
            }
        }

        close(p[numPipes-1][0]);
        close(p[numPipes-1][1]);

        for (int i = 0; i < numPipes; i++) {
            wait(NULL);
        }

        free(p);
    }
}

int main(){

    FILE* file = fopen("config.txt", "r");
    
    if (file == NULL){
        printf("Error abriendo config.txt");
    }

    fav_file_route = malloc(256);

    fgets(fav_file_route, 256, file);

    fclose(file);

    char command[1024];
    char *pipeArgs[64];
    char* ands[64];

    int fav_index = 0;

    pid_t pid;

    while(1){

        printf(">");

        fgets(command, sizeof(command), stdin);

        char* buscar = check_buscar(command);
        long int* eliminar = check_eliminar(command);

        if(strcmp(command, "exit\n") == 0){
            break;
        }

        if (strcmp(command, "favs mostrar\n") == 0){
            favs_mostrar();
        }
        else if (strcmp(command, "favs cargar\n") == 0){
            favs_cargar();
        }
        else if (strcmp(command, "favs borrar\n") == 0) {
            favs_borrar();
        }
        else if (strcmp(command, "favs guardar\n") == 0){
            favs_guardar();
        }
        else if (check_crear(command)){
            favs_crear();
        }
        else if (buscar != NULL){
            favs_buscar(buscar);
        }
        else if (eliminar != NULL){
            favs_eliminar(eliminar);
        }
        else{
            
            free(eliminar);

            char* ejecutar = favs_ejecutar(command);

            if (ejecutar != NULL){  //Si el comando es favs ejecura str, cambia command por str
                strcpy(command, ejecutar);
            }

            if (check_set_recordatorio(command)){ 
                set_recordatorio(command);
                continue;
            }
            
            if (!check_repeated(command) && strcmp(command,"") && strcmp(command,"\n")){
                fav_list[fav_index] = strdup(command); 
                fav_index++;
            }

            int numANDS = parse_and(command, ands); //Se parsean los posibles && 

            for (int i = 0; i < numANDS; i++){

                int numPipes = parse_pipe(ands[i], pipeArgs); //Se parsean los posibles pipes
                
                pid = fork();

                if (pid == 0){

                    exec_(pipeArgs, numPipes);  //Función para generalizar los exec()
                    exit(EXIT_FAILURE);
                    
                } else if ( pid < 0){ 
                    printf("Error\n");
                }
                else
                    wait(NULL);    

            }
        }
    }

    return 0;
}
