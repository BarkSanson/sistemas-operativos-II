#include "directorios.h"

int main(int argc, char** argv) {
    char* camino;
    char* disco;
    char permisos = -1;

    if(argc != 4) {
        fprintf(stderr, "mi_ls: Sintaxis incorrecta, la sintaxis debe seguir la siguiente forma:\n\t ./mi_mkdir <disco> <permisos> <ruta>\n");
        return 1;
    }

    disco = argv[1];
    permisos += atoi(argv[2]);
    camino = argv[3];

    // Si permisos sigue siendo -1, es porque
    // atoi no ha podido realizar la conversion
    if(permisos == -1) {
        fprintf(stderr, "mi_ls: los permisos introducidos no son válidos.\n");
        return 1;
    }

    permisos++;

    if((permisos < 0 && permisos > 7)) {
        fprintf(stderr, "mi_ls: los permisos deben estar entre 0 y 7\n");
        return 1;
    }

    bmount(disco);

    mi_creat(camino, permisos);

    bumount();

    return 0;
}