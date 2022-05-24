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

    bmount(disco);

    if(mi_unlink(ruta) == ERROR_EXIT) {
        fprintf(
            stderr, 
            "mi_rm: no se ha podido eliminar el fichero/directorio correctamente\n");
        return 1;
    }

    bumount();

    return 0;
}