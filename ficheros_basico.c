#include "ficheros_basico.h"

int tamMB(unsigned int nbloques) {
    // Calculamos el tamaño del MB con
    // la división entera
    int tam = (nbloques / 8) / BLOCKSIZE;

    // Si el módulo es mayor que 0,
    // significa que necesitamos un bloque
    // más
    if(((nbloques / 8) % BLOCKSIZE) > 0) {
        tam++;
    }

    return tam;
}

int tamAI(unsigned int ninodos) {
    int tam = 0;
    
    //sabiendo que el numero de inodos se ha calculado
    //ya de forma heurística el tamaño de este bloque será

    tam = (ninodos*INODOSIZE)/BLOCKSIZE;

    //vemos si necesitamos un bloque más o no

    if(((ninodos*INODOSIZE) % BLOCKSIZE) != 0){
        tam++;
    }

    return tam;
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

int initMB() {
    struct superbloque* SB;
    unsigned char buf[BLOCKSIZE];

    SB = malloc(sizeof(struct superbloque));

    if(bread(posSB, SB) == EXIT_FAILURE) return EXIT_FAILURE;

    int bloquesMB = SB->posUltimoBloqueMB - SB->posPrimerBloqueMB;
    int bloquesMetadatos = SB->posUltimoBloqueAI + 1;

    //Ponemos todos los bloques del
    //MB a 0, nos posicionamos en el inicio
    //del bloque de MB
    memset(buf, 0, BLOCKSIZE);
    for(int i = 1; i<bloquesMB; i++){
        if(bwrite(i, buf) == EXIT_FAILURE) return EXIT_FAILURE;
    }

    //ahora ponemos los bits a 1 del
    //mapa de bits que se corresponden con metadatos
    memset(buf,1,bloquesMetadatos);
    if(bwrite(posSB, buf) == EXIT_FAILURE) return EXIT_FAILURE;

    // Ya que hemos marcado en el mapa de bits los bloques
    // ya ocupados, tenemos que actualizar la cantidad 
    // de bloques libres en el superbloque
    SB->cantBloquesLibres = SB->cantBloquesLibres - bloquesMetadatos;
    if(bwrite(posSB, SB) == EXIT_FAILURE) return EXIT_FAILURE;

    free(SB);

    return EXIT_SUCCESS;
}

/**
 * int initAI()
 * Inicializa todos los inodos del dispositivo
 * returns: EXIT_FAILURE si ha habido algun error,
 *          EXIT_SUCCESS de lo contrario
 */
int initAI() {
    struct superbloque* SB;
    struct inodo inodos[BLOCKSIZE/INODOSIZE];

    SB = malloc(sizeof(struct superbloque));
    // Leemos el superbloque
    if(bread(posSB, SB) == EXIT_FAILURE) return EXIT_FAILURE;

    int contInodos = SB->posPrimerInodoLibre + 1;

    // Recorremos todos los bloques de inodos
    for(int i = SB->posPrimerBloqueAI; i <= SB->posUltimoBloqueAI; i++) {
        // Leemos un bloque
        if(bread(i, inodos) == EXIT_FAILURE) return EXIT_FAILURE;

        // Por cada inodo del bloque, lo inicializamos
        // como libre y lo enlazamos con el siguiente
        for(int j = 0; j < BLOCKSIZE / INODOSIZE; j++) {
            inodos[j].tipo = 'l';
            if(contInodos < SB->totInodos) {
                inodos[j].punterosDirectos[0] = contInodos;
                contInodos++;
            } else {
                inodos[j].punterosDirectos[0] = UINT_MAX;
                break;
            }
        }

        // Escribimos las modificaciones en el
        // dispositivo virtual
        if(bwrite(i, inodos) == EXIT_FAILURE) return EXIT_FAILURE;
    }

    free(SB);

    return EXIT_SUCCESS;
}