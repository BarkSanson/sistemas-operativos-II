#include "directorios.h"

int main(int argc, char** argv) {
    char* disco;
    char* original;
    char* nueva_ruta;

    if(argc != 4) {
        fprintf(
            stderr, 
            "mi_link: sintaxis incorrecta, la sintaxis debe seguir la siguiente forma:\n\t ./mi_link <disco> <ruta_original> <ruta_nueva>\n");
        return 1;
    }

    disco = argv[1];
    original = argv[2];
    nueva_ruta = argv[3];

    // Si el camino no tiene / al final,
    // no es un directorio.
    if(*(original + strlen(original) - 1) == '/' || *(nueva_ruta + strlen(nueva_ruta) - 1) == '/') {
        fprintf(stderr, "mi_link: uno o ambos caminos introducidos no corresponden a un fichero\n");
        return 1;
    }
    bmount(disco);

    if(mi_link(original, nueva_ruta) == ERROR_EXIT) {
        return 1;
    }

    bumount();

    return 0;
}