#include "fav_commands.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int check_crear(char* command){

    char* args[100];

    char aux[1024];
    strcpy(aux, command);
    parse_command(aux, args);
    
    if (args[0] == NULL || args[1] == NULL || args[2] == NULL || args[3] != NULL){
        return 0;
    }

    int len = strlen(args[2]);

    if (strcmp(args[0],"favs") || strcmp(args[1], "crear") || strcmp(args[2] + len - 4, ".txt")){
        return 0;
    }


    fav_file_route = strdup(args[2]);
    
    return 1;

}

void favs_crear(){

    char* args[3];
    args[0] = "touch";
    args[1] = strdup(fav_file_route);
    args[2] = NULL;
 

    if (fork() == 0){

        if (execvp(args[0], args) == -1){
            printf("Error creando el archivo\n");
            return;
        }
    }

    FILE* file = fopen("config.txt", "w");

    if (file == NULL){
        printf("Error abriendo config.txt\n");
        return;
    }

    fprintf(file, "%s", fav_file_route);

    fclose(file);

}

char* favs_ejecutar(char* command){

    char* args[100];

    char aux[1024];
    strcpy(aux, command);
    parse_command(aux, args);
    
    char* endptr;
    long int num;

    if (args[0] == NULL || args[1] == NULL || args[2] == NULL || args[3] != NULL){
        return NULL;
    }

    num = strtol(args[1], &endptr, 10); 
    
    if (strcmp(args[0],"favs") || endptr == args[1] || *endptr != '\0' || strcmp(args[2],"ejecutar"))
        return NULL;

    return fav_list[num];

}

void favs_mostrar(){

    int i = 0;
    if (fav_list != NULL){
        while (fav_list[i] != NULL){
            if (fav_list[i] != "\0")
                printf(" ID %d: %s", i, fav_list[i]);
            i++;
        }
    }
    else{
        printf("favs vacio\n");
    }
}

void favs_borrar(){
    int i = 0;
    while ( fav_list[i] != NULL){
        fav_list[i] = NULL;
        i++;
    }
}

long int* check_eliminar(char* command){

    char* args[100];

    char aux[1024];
    strcpy(aux, command);
    parse_command(aux, args);
    
    char* endptr;

    if (args[0] == NULL || args[1] == NULL || args[2] == NULL || args[3] != NULL){
        return NULL;
    }

    if (strcmp(args[0],"favs") || strcmp(args[1],"eliminar"))
        return NULL;

    int i = 0;
    char* token = strtok(args[2], ",");

    char* numStr[64];

    while (token != NULL){
        numStr[i++] = token;
        token = strtok(NULL, ",");
    }

    numStr[i] = NULL;

    long int* nums = malloc((i+1)*sizeof(long int));
    int j = 0;

    while (numStr[j] != NULL){
        nums[j] = strtol(numStr[j], &endptr, 10);
        j++;
    }
    
    nums[j] = -1;

    return nums;

}

void favs_eliminar(long int* nums){

    int i = 0;
    
    while (nums[i] != -1){

        fav_list[nums[i]] = "\0";
        i++;
    }

}

char* check_buscar(char* command){

    char* args[100];

    char aux[1024];
    strcpy(aux, command);
    parse_command(aux, args);

    if (args[0] == NULL || args[1] == NULL || args[2] == NULL || args[3] != NULL){
        return NULL;
    }
    
    if (strcmp(args[0],"favs") || strcmp(args[1],"buscar"))
        return NULL;

    return args[2];

}

void favs_buscar(char* cmd){

    int i = 0;

    while (fav_list[i] != NULL){
        char* p = strstr(fav_list[i], cmd);
        if (p)
            printf(" ID %d: %s", i, fav_list[i]);
        i++;
    }

}

int check_repeated(char* string){

    int i = 0;

    while (fav_list[i] != NULL){
        if (strcmp(fav_list[i], string) == 0){
            return 1;
        }
        i++;
    }

    return 0;
}

void favs_cargar(){

    FILE* file = fopen(fav_file_route, "r");

    if (file == NULL){
        printf("Error abriendo el archivo de favoritos\n");
        return;
    }

    char buffer[256];
    int file_line_index = 0;
    int fav_list_index = 0;

    while (fav_list[fav_list_index] != NULL){
        fav_list_index++;
    }


    while (fgets(buffer, sizeof(buffer), file)){

        if (!check_repeated(buffer)){
            fav_list[fav_list_index] = strdup(buffer);
            fav_list_index++;
        }

        file_line_index++;
    }

    fclose(file);

}

void favs_guardar(){

    FILE* file = fopen(fav_file_route, "a+");

    if (file == NULL){
        printf("Error abriendo el archivo de favoritos\n");
        return;
    }

    int i = 0;
    int isIn = 0;
    char buffer[256];

    size_t sizeBuffer= sizeof(buffer);
    while (fav_list[i] != NULL){
        while (fgets(buffer, sizeBuffer, file)){
            if (strcmp(buffer, fav_list[i]) == 0){
                isIn = 1;
                break;
            }
        }

        if (!isIn && (strcmp(fav_list[i], "\0") != 0)){
            fprintf(file, "%s", fav_list[i]);    
        }
        i++;
    }

    fclose(file);

}

int check_set_recordatorio(char* command){
    char* args[100];

    char aux[1024];
    strcpy(aux, command);
    parse_command(aux, args);

    if (args[0] == NULL || args[1] == NULL || args[2] == NULL || args[3] == NULL || args[4] != NULL){
        return 0;
    }

    if (strcmp(args[0],"set") || strcmp(args[1],"recordatorio"))
        return 0;

    return 1;
}

void set_recordatorio(char* command){

    char* args[100];

    char aux[1024];
    strcpy(aux, command);
    parse_command(aux, args);

    long int sec;
    char* endptr;

    sec = strtol(strdup(args[2]), &endptr, 10);

    if (fork() == 0){
        sleep(sec);
        printf("%s\n", strdup(args[3]));
        exit(0);
    }
}