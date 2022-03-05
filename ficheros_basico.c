#include "ficheros_basico.h"

int nbloques = 0; //!!!

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

int initMB(){
    int bloquesMB = tamMB(nbloques);
    int bloquesMetadatos = tamAI(nbloques) + bloquesMB + tamSB;
    unsigned char buf[BLOCKSIZE];

    //Ponemos todos los bloques del
    //MB a 0, nos posicionamos en el inicio
    //del bloque de MB
    memset(buf,0,BLOCKSIZE);
    for(int i=0; i<bloquesMB; i++){
        bwrite(i+posSB, buf);
    }

    //ahora ponemos los bits a 1 del
    //mapa de bits que se corresponden con metadatos
    memset(buf,1,bloquesMetadatos);
    bwrite(posSB, buf);

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

    return EXIT_SUCCESS;
}