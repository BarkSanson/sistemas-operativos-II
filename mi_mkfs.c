#include "bloques.h"

int main(int argc, char **argv) {
    char *path;
    int nbloques;
    unsigned char buf[BLOCKSIZE];

    if(argc != 3) return 1;

    // Montamos el dispositivo virtual
    path = argv[2];
    bmount(path);

    // Escribimos los n bloques en el fichero
    // inicializados a 0
    nbloques = atoi(argv[2]);
    if(nbloques == 0) {
        fprintf(stderr, "Numero de bloques invalido");
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