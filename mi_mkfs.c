#include "bloques.h"

int main(int argc, char **argv) {
    char *path;
    int nbloques;
    unsigned char buf[BLOCKSIZE];

    if(argc != 3) {
        fprintf(stderr, "Número de argumentos inválido: son necesarios 2 argumentos con la forma \n\tmi_mkfs {path} {número de bloques a escribir}");
        return 1;
    }

    // Montamos el dispositivo virtual
    path = argv[1];
    bmount(path);

    // Escribimos los n bloques en el fichero
    // inicializados a 0
    nbloques = atoi(argv[2]);
    if(nbloques == 0) {
        fprintf(stderr, "Número de bloques inválido");
        return 1;
    }

    memset(buf, 0, BLOCKSIZE);

    for(int i = 0 ; i < nbloques; i++) {
        bwrite(i, buf);
    }

    // Desmontamos el dispositivo virtual
    bumount();

    return 0;
}