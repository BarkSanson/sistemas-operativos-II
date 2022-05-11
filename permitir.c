/**
 * 
 * Autores: Arnau Vidal Moreno y Martín Ignacio Rizzo
 */ 
#include "ficheros.h"
#include "string.h"

int main(int argc, char** argv) {
    char *path;
    char permisos;
    int ninodo;

    if(argc != 4) {
        fprintf(stderr, "%sNúmero de argumentos inválido, son necesarios 3 argumentos:%s\n\tpermitir <nombre_dispositivo> <número_de_inodo> <permisos_inodo>\n",
        RED,
        RESET_COLOR);
        return EXIT_FAILURE;
    }

    //montamos el dispositivo 
    path = argv[1];
    ninodo = atoi(argv[2]);
    permisos = atoi(argv[3]);

    bmount(path);

    //llamamos a la funcion mi_chmod_f dandole el numero de inodo
    //y sus correspondientes permisos
    if(mi_chmod_f(ninodo, permisos) == ERROR_EXIT) {
        fprintf(stderr, "permitir.c: No se han podido modificar los eprsmisos del inodo %d", 
        ninodo);
    }

    //desmontamos el dispositivo
    bumount();

    return 0;
}