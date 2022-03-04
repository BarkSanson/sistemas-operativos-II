#include "ficheros_basico.h"

int tamMB(unsigned int nbloques) {
    // Calculamos el tamaño del MB con
    // la división entera
    int tam = (nbloques / 8) / BLOCKSIZE;

    // Si el módulo es mayor que 0,
    // significa que necesitamos un bloque
    // más
    if(((nbloques / 8) / BLOCKSIZE) > 0) {
        tam++;
    }

    return tam;
}

int tamAI(unsigned int ninodos) {
    // TODO
    return 0;
}

int initSB(unsigned int nbloques, unsigned int ninodos) {
    struct superbloque SB;

    // Inicializamos todos los campos
    SB.posPrimerBloqueMB = posSB + tamSB;
    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;
    SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;
    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
    SB.posUltimoBloqueDatos = nbloques - 1;
    SB.posPrimerInodoLibre = 0; // Cambiará en el Nivel 3
    SB.cantBloquesLibres = nbloques;
    SB.cantInodosLibres = ninodos;
    SB.totBloques = nbloques;
    SB.totInodos = ninodos;

    // Escribimos el superbloque en su posición y
    // comprobamos si ha habido algun error
    if(bwrite(posSB, &SB) == EXIT_FAILURE) return EXIT_FAILURE;

    return EXIT_SUCCESS;
}