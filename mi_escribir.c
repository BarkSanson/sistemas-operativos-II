#include "directorios.h"

int main(int argc, char** argv) {
    char* disco;     
    char* ruta;     
    char* texto;     
    int offset; 
    int bytesEscritos = 0;

    if(argc != 5) {
        fprintf(stderr, 
        "mi_escribir: sintaxis incorrecta, la sintaxis debe seguir la siguiente forma:\n\t ./mi_escribir <disco> <ruta> <texto> <offset>\n");
        return 1;
    }

    disco = argv[1];
    ruta = argv[2];
    texto = argv[3];

    offset = ERROR_EXIT;
    offset += atoi(argv[4]);

    if(offset == ERROR_EXIT) {
        fprintf(stderr,
        "mi_escribir: el offset introducido no es válido");
        return 1;
    }

    offset++;

    if(*(ruta + strlen(ruta) - 1) == '/') {
        fprintf(stderr,
        "mi_escribir: la ruta introducida no es la de un fichero");
        return 1;
    }

    bmount(disco);

    bytesEscritos = mi_write(ruta, texto, offset, strlen(texto));
    if(bytesEscritos == ERROR_EXIT) {
        fprintf(stderr,
        "mi_escribir: no se han podido escribir los datos en el fichero");
        return 1;
    }

    printf("\nBytes escritos: %d\n", bytesEscritos);

    bumount();

    return 0;
}