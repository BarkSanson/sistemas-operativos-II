/**
 * Autores: Arnau Vidal Moreno y Martín Ignacio Rizzo
 */ 
#include "directorios.h"

int main(int argc, char **argv) {
    char *path;
    int nbloques;
    unsigned int tamDV;
    FILE* fp;

    if(argc != 3) {
        fprintf(stderr, "Número de argumentos inválido: son necesarios 2 argumentos con la forma \n\tmi_mkfs {path} {número de bloques a escribir}");
        return 1;
    }

    // Montamos el dispositivo virtual
    path = argv[1];

    tamDV = atoi(argv[2]) * BLOCKSIZE - 1;
    fp = fopen(path, "w");
    fseek(fp, tamDV, SEEK_SET);
    fputc('\0', fp);
    fclose(fp);

    bmount(path);

    nbloques = atoi(argv[2]);

    initSB(nbloques, nbloques/4);
    initMB();
    initAI();

    reservar_inodo('d', 7);

    // Desmontamos el dispositivo virtual
    bumount();

    return 0;
}