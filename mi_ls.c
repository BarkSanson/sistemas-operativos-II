#include "directorios.h"

int main(int argc, char** argv) {
    char buffer[TAMBUFFER];
    char* disco;
    char* camino;

    if(argc != 3) {
        fprintf(stderr, "mi_ls: Sintaxis incorrecta, la sintaxis debe seguir la siguiente forma:\n\t ./mi_ls <disco> <ruta_directorio>");
        return 1;
    }

    disco = argv[1];
    camino = argv[2];

    bmount(disco);

    if(mi_dir(camino, buffer) == ERROR_EXIT) {
        return 1;
    }

    printf("Nombre\tTipo\tPermisos\tTamaño\tmtime\n");
    printf("------------------------------------------------------\n");
    printf(buffer);

    bumount();

    return 0;
}