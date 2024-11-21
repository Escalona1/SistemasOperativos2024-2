#ifndef FAV_COMMANDS_H
#define FAV_COMMANDS_H

extern char* fav_list[100];
extern char* fav_file_route;
extern int parse_command(char* command, char** args);
extern void exec_(char** pipeArgs, int numPipes);

extern void print_args(char **args);

int check_crear(char* command);
void favs_crear();

char* check_buscar(char* command);
void favs_buscar(char* cmd);

long int* check_eliminar(char* command);
void favs_eliminar(long int* nums);

char* favs_ejecutar(char* command);

void favs_mostrar();
void favs_borrar();
void favs_cargar();
void favs_guardar();

int check_repeated(char* string);

int check_set_recordatorio(char* command);
void set_recordatorio(char* command);

#endif