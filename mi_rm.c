/**
 * Autores: Arnau Vidal Moreno y Martín Ignacio Rizzo
 */ 
#include "directorios.h"

int main(int argc, char** argv) {
    char* disco;
    char* ruta;

    if(argc != 3) {
        fprintf(
            stderr, 
            "mi_rm: sintaxis incorrecta, la sintaxis debe seguir la siguiente forma:\n\t ./mi_rm <disco> <ruta>\n");
        return 1;
    }

    disco = argv[1];
    ruta = argv[2];

    if(*(ruta + strlen(ruta) - 1) == '/') {
        fprintf(
            stderr, 
            "mi_rm: la ruta introducida no pertenece a un fichero\n");
        return 1;
    }

    bmount(disco);

    if(mi_unlink(ruta) == ERROR_EXIT) {
        return 1;
    }

    bumount();

    return 0;
}