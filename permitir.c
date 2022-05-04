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
        fprintf(stderr, "%sNúmero de argumentos inválido, son necesarios 3 argumentos:%s\n\tpermitir <nombre_dispositivo> <número_de_inodo> <permisos_inodo>",
        RED,
        RESET_COLOR);
        return EXIT_FAILURE;
    }

    printf("%sEJECUTANDO TEST PERMITIR.C%s\n", BOLD_GREEN, RESET_COLOR);
     
    //montamos el dispositivo 
    path = argv[1];
    bmount(path);

    //transformamos a int el numero de inodo
    ninodo = atoi(argv[2]);
    strcpy(argv[3],&permisos);
    //llamamos a la funcion mi_chmod_f dandole el numero de inodo
    //y sus correspondientes permisos
    mi_chmod_f(ninodo,permisos);

    //desmontamos el dispositivo
    bumount();

    return 0;
}