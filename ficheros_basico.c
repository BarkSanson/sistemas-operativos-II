#include "ficheros_basico.h"

/**
 * Calcula el tamaño en bloques del mapa de bits
 * 
 * @param   nbloques    Numero de bloques totales del sistema
 * @returns             Tamaño en bloques del mapa de bits 
 *                      
 */
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

/**
 * Calcula el tamaño en bloques del array de inodos 
 * 
 * @param   ninodos     Numero de inodos del sistema. En este caso, nbloques/4
 * @returns             Tamaño en bloques del array de inodos 
 *                      
 */
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

/**
 * Inicializa el superbloque
 * 
 * @param   nbloques    Numero de bloques totales del sistema
 * @param   ninodos     Numero de inodos del sistema. En este caso, nbloques/4
 * @returns             EXIT_FAILURE si ha habido algun error,
 *                      EXIT_SUCCESS de lo contrario
 */
int initSB(unsigned int nbloques, unsigned int ninodos) {
    struct superbloque SB;

    // Inicializamos todos los campos
    SB.posPrimerBloqueMB = posSB + tamSB;
    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;
    SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;
    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
    SB.posUltimoBloqueDatos = nbloques - 1;
    SB.posInodoRaiz = 0;
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

/**
 * Inicializa el mapa de bits
 * 
 * @returns:        EXIT_FAILURE si ha habido algun error,
 *                  EXIT_SUCCESS de lo contrario
 */
int initMB() {
    struct superbloque* SB;
    unsigned char buf[BLOCKSIZE];
    int bloquesMetadatos;
    int bloquesNecesarios;
    int bytesNecesarios;
    int posBloque = 1;

    SB = malloc(sizeof(struct superbloque));

    if(bread(posSB, SB) == EXIT_FAILURE) return EXIT_FAILURE;

    // Sabiendo que los bloques de los metadatos estarán seguidos,
    // el total de bloques que estos ocuparán será:
    bloquesMetadatos = SB->posUltimoBloqueAI + 1;

    bloquesNecesarios = (bloquesMetadatos / 8) % BLOCKSIZE == 0 ? 
                            (bloquesMetadatos / 8) / BLOCKSIZE : 
                            ((bloquesMetadatos / 8) / BLOCKSIZE) + 1;
    bytesNecesarios = bloquesMetadatos / 8; 

    // Si necesitamos más de un bloque, podemos
    // escribir directamente a 1 los bloques enteros
    if(bloquesNecesarios > 1) {
        memset(buf, 255, BLOCKSIZE);
        for(int i = 1; i < bloquesNecesarios; i++) {
            if(bwrite(posBloque++, buf) == EXIT_FAILURE) return EXIT_FAILURE;
        }
        bytesNecesarios -= (bloquesNecesarios - 1) * BLOCKSIZE;
    }

    // Reseteamos el buffer
    memset(buf, 0, BLOCKSIZE);

    // Todos los bytes que vayan completos
    // los escribimos todos a 1
    for(int i = 0; i < bytesNecesarios; i++) {
        buf[i] = 255;
    }

    // Si hay bits que nos han quedado sueltos, tenemos
    // que escribirlos en el siguiente byte
    if(bloquesMetadatos % 8 != 0) {
        int bitsSueltos = bloquesMetadatos % 8;
        char valorBitsSueltos = 0;
        for(int i = 7; bitsSueltos > 0; i--) {
            valorBitsSueltos += (int) pow(2/1.0, i/1.0);
            bitsSueltos--;
        }
        buf[bytesNecesarios] = valorBitsSueltos;
    }

    // Rellenamos los bytes que sobren a 0
    for(int i = bytesNecesarios; i < BLOCKSIZE; i++) {
        buf[i] = 0;
    }

    // Escribimos el buffer
    if(bwrite(posBloque, buf) == EXIT_FAILURE) return EXIT_FAILURE;

    // Ya que hemos marcado en el mapa de bits los bloques
    // ya ocupados, tenemos que actualizar la cantidad 
    // de bloques libres en el superbloque
    SB->cantBloquesLibres = SB->cantBloquesLibres - bloquesMetadatos;
    if(bwrite(posSB, SB) == EXIT_FAILURE) return EXIT_FAILURE;

    free(SB);

    return EXIT_SUCCESS;
}

/**
 * Inicializa todos los inodos del dispositivo
 * 
 * @returns:        EXIT_FAILURE si ha habido algun error,
 *                  EXIT_SUCCESS de lo contrario
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