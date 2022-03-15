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
 * @returns         EXIT_FAILURE si ha habido algun error,
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
 * @returns         EXIT_FAILURE si ha habido algun error,
 *                  EXIT_SUCCESS de lo contrario
 */
int initAI() {
    struct superbloque* SB;
    struct inodo inodos[BLOCKSIZE/INODOSIZE];

    SB = malloc(sizeof(struct superbloque));
    // Leemos el superbloque
    if(bread(posSB, SB) == EXIT_FAILURE) {
        free(SB);
        return EXIT_FAILURE;
    }

    int contInodos = SB->posPrimerInodoLibre + 1;

    // Recorremos todos los bloques de inodos
    for(int i = SB->posPrimerBloqueAI; i <= SB->posUltimoBloqueAI; i++) {

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
        if(bwrite(i, inodos) == EXIT_FAILURE) {
            free(SB);
            return EXIT_FAILURE;
        }
    }

    free(SB);

    return EXIT_SUCCESS;
}

/**
 * Escribe un valor determinado dado por parámetro
 * en un bit del mapa de bits
 * 
 * @param   nbloque numero de bloque a cambiar su valor en el MB
 * @param   bit     valor a actualizar el bloque
 * @return          devuelve EXIT_FAILURE si da error, o EXIT_SUCCESS
 *                  si se ha escrito de forma correcta
 */
int escribir_bit(unsigned int nbloque, unsigned int bit){
    //declaramos SB y leemos los valores del mismo
    struct superbloque* SB;
    SB = malloc(sizeof(struct superbloque));
    if(bread(posSB, SB) == EXIT_FAILURE){
        free(SB);
        return EXIT_FAILURE;
    }
    //en que bloque del MB estamos (ABSOLUTO)
    int nbloqueMB = nbloque/BLOCKSIZE;
    int nbloqueabs = SB->posPrimerBloqueMB + nbloqueMB;
    //en que byte del bloque estamos y lo mismo con el bit
    int posByte = nbloque/8;
    posByte = posByte % BLOCKSIZE;
    int posbit = nbloque%8;

    //buffer donde leeremos el bloque del mapa de bits
    unsigned char bufferMB[BLOCKSIZE];
    bread(nbloqueabs, bufferMB);
    //preparamos el byte a poner en uno
    unsigned char mascara = 1000000;
    mascara >>= posbit;

    //en funcion del caso marcamos como escrito
    //o como borrado
    if(bit == 1){
        bufferMB[posByte] |= mascara;
    } else {
        bufferMB[posByte] &= ~mascara;
    }

    if(bwrite(nbloqueabs,bufferMB) == EXIT_FAILURE){
        free(SB);
        return EXIT_FAILURE;
    } else {
        free(SB);
        return EXIT_SUCCESS;
    }
}
/**
 * Lee el valor que representa un bloque en nuestro MB
 * 
 * @param   nbloque bloque a leer en el MB
 * @returns         devuelve EXIT_FAILURE si ha habido un error
 *                  en el proceso de lectura, o el valor en el que se encuentra el estado
 *                  de nuestro bloque en el MB
 */
char leer_bit(unsigned int nbloque){
    //declaramos SB y leemos los valores del mismo
    struct superbloque* SB;
    SB = malloc(sizeof(struct superbloque));
    if(bread(posSB, SB) == EXIT_FAILURE){
        free(SB);
        return EXIT_FAILURE;
    }
    //en que bloque del MB estamos (ABSOLUTO)
    int nbloqueMB = nbloque/BLOCKSIZE;
    int nbloqueabs = SB->posPrimerBloqueMB + nbloqueMB;
    //en que byte del bloque estamos y lo mismo con el bit
    int posByte = nbloque/8;
    posByte = posByte % BLOCKSIZE;
    int posbit = nbloque%8;

    //buffer donde leeremos el bloque del mapa de bits
    unsigned char bufferMB[BLOCKSIZE];
    if(bread(nbloqueabs, bufferMB) == EXIT_FAILURE){
        free(SB);
        return EXIT_FAILURE;
    }

    unsigned char mascara = 128; 
    mascara >>= posbit;          
    mascara &= bufferMB[posByte];
    mascara >>= (7 - posbit);

    free(SB);
    return mascara;  
}

/**
 * Reservamos el primer bloque libre de nuestro MB para
 * 
 * @returns devuelve EXIT_FAILURE si ha habido un error en su ejecución
 *          o el numero del primer bloque libre en nuestro sistema
 */
int reservar_bloque(){
    //reservamos memoria para el superbloque y lo leemos
    struct superbloque* SB;
    SB = malloc(sizeof(struct superbloque));
    if(bread(posSB, SB) == EXIT_FAILURE){
        return EXIT_FAILURE;
    }
    //miramos si queda algun bloque libre
    if(SB->cantBloquesLibres == 0){
        printf("No quedan bloques libres");
        return EXIT_FAILURE;
    }

    unsigned char buffer[BLOCKSIZE];
    unsigned char aux[BLOCKSIZE];
    unsigned char mascara = 128;
    memset(aux,255,BLOCKSIZE);
    //conseguimos el primer bloque de MB
    int nbloqueabs = SB->posPrimerBloqueMB;
    //y los bloques a recorrer, lo hacemos asi para ahorrar
    //uso de la cpu invocando a tamMB()
    int tamMB = SB->posPrimerBloqueAI - SB->posPrimerBloqueMB;

    //vamos comparando todos los blqoues a ver si hay algoç
    //en alguno de ellos
    int posbit = 0;
    int nbloque = 0;
    for(int i=0; i<tamMB; i++){
        bread(nbloqueabs + i , buffer);
        //si son diferentes quiere decir que habrá un bit
        //libre en el bloque traido del MB
        if(memcmp(buffer,aux,BLOCKSIZE) != 0){
            //recorremos ahora todo el bloque en busca de un bit libre
            for(int posByte=0;i<BLOCKSIZE;posByte++){
                //el byte tiene al menos un bit vacío
                if(buffer[posByte] != 255){
                    while(buffer[posByte] & mascara){
                        buffer[posByte] <<= 1;
                        posbit++;
                    }
                    //encontramos el numero de bloque definitivo
                    nbloque = ((nbloqueabs - SB->posPrimerBloqueMB)*BLOCKSIZE+posByte)*8 + posbit;
                    if(escribir_bit(nbloque,1) == EXIT_FAILURE){
                        free(SB);
                        return EXIT_FAILURE;
                    }
                    SB->cantBloquesLibres--;
                    //reseteamos el bloque a reservar por si había datos
                    memset(aux,0,BLOCKSIZE);
                    if(bwrite(nbloque,aux) == EXIT_FAILURE){
                        free(SB);
                        return EXIT_FAILURE;
                    }
                    free(SB);
                    return nbloque;
                }
            }
        }
    }
    //si ha llegado hasta aqui es porque no hay bloques libres
    //y por ende no se puede reservar ningún bloque
    free(SB);
    return EXIT_FAILURE;
}

// TODO
int liberar_bloque(unsigned int nbloque);

int escribir_inodo(unsigned int ninodo, struct inodo* inodo) {
    struct superbloque* SB;
    struct inodo* inodos;
    unsigned char buf[BLOCKSIZE];
    int bloqueRelativoInodo;

    SB = malloc(sizeof(struct superbloque));

    // Calculamos el número relativo de bloque en
    // el que se encuentra el inodo solicitado
    bloqueRelativoInodo = ninodo/(BLOCKSIZE/INODOSIZE);
    if(ninodo%(BLOCKSIZE/INODOSIZE) != 0) bloqueRelativoInodo++;

    // Leemos el bloque solicitado
    if(bread(SB->posPrimerBloqueAI + bloqueRelativoInodo, buf) == EXIT_FAILURE) {
        free(SB);
        return EXIT_FAILURE;
    }

    inodos = bloque_a_inodos(buf);





    free(SB);
}

/**
 * Pasa un bloque de inodos a un array de inodos
 * 
 * @param   buf     Buffer que contiene un bloque de inodos
 * @returns         Puntero al array de inodos
 * 
 */
struct inodo* bloque_a_inodos(const unsigned char* buf) {
    struct inodo inodos[BLOCKSIZE/INODOSIZE];
    for(int i = 0, j = INODOSIZE; i < BLOCKSIZE/INODOSIZE; i = j + 1, j += INODOSIZE) {
        memcpy(inodos + i, buf + i, INODOSIZE);
    }

    return inodos;
}