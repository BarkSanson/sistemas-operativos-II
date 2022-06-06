/**
 * Autores: Arnau Vidal Moreno y Martín Ignacio Rizzo
 */ 
#include "directorios.h"

#define TAM_BUFFER 1500

int main(int argc, char** argv) {
    char* disco;     
    char* ruta;     
    int bytesTotalesLeidos = 0;
    int bytesLeidosIter = 0;
    int offset = 0;
    char buffer[TAM_BUFFER];

    if(argc != 3) {
        fprintf(stderr, 
        "mi_cat: sintaxis incorrecta, la sintaxis debe seguir la siguiente forma:\n\t ./mi_cat <disco> <ruta>\n");
        return 1;
    }

    disco = argv[1];
    ruta = argv[2];

    if(*(ruta + strlen(ruta) - 1) == '/') {
        fprintf(stderr,
        "mi_cat: la ruta introducida no es la de un fichero\n");
        return 1;
    }

    bmount(disco);

    // Limpiamos completamente el buffer
    memset(buffer, 0, sizeof(buffer));

    // Tenemos que leer el inodo entero de forma secuencial
    bytesLeidosIter = mi_read(ruta, buffer, 0, TAM_BUFFER);
    while(bytesLeidosIter > 0) {
        bytesTotalesLeidos += bytesLeidosIter;

        // Imprimimos lo leído por pantalla
        write(1, buffer, bytesLeidosIter);

        // Limpiamos el buffer a cada iteración,
        // para que no haya basura
        memset(buffer, 0, sizeof(buffer));
        
        offset += bytesLeidosIter;

        bytesLeidosIter = mi_read(ruta, buffer, offset, TAM_BUFFER);
    }

    fprintf(stderr, "\nBytes leidos: %d\n", bytesTotalesLeidos);

    bumount();

    return 0;
}