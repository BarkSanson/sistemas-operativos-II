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
 * @returns             ERROR_EXIT si ha habido algun error,
 *                      SUCCESS_EXIT de lo contrario
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
    if(bwrite(posSB, &SB) == ERROR_EXIT) return EXIT_FAILURE;

    return SUCCESS_EXIT;
}

/**
 * Inicializa el mapa de bits
 * 
 * @returns         ERROR_EXIT si ha habido algun error,
 *                  SUCCESS_EXIT de lo contrario
 */
int initMB() {
    struct superbloque SB;
    unsigned char buf[BLOCKSIZE];
    int bloquesMetadatos;
    int bloquesNecesarios;
    int bytesNecesarios;
    int posBloque = 1;

    if(bread(posSB, &SB) == ERROR_EXIT) {
        fprintf(stderr, 
                "[Error en initMB()]: No se ha podido leer el superbloque");
        return ERROR_EXIT;
    }
    // Sabiendo que los bloques de los metadatos estarán seguidos,
    // el total de bloques que estos ocuparán será:
    bloquesMetadatos = SB.posUltimoBloqueAI + 1;

    bloquesNecesarios = (bloquesMetadatos / 8) % BLOCKSIZE == 0 ? 
                            (bloquesMetadatos / 8) / BLOCKSIZE : 
                            ((bloquesMetadatos / 8) / BLOCKSIZE) + 1;
    bytesNecesarios = bloquesMetadatos / 8; 

    // Si necesitamos más de un bloque, podemos
    // escribir directamente a 1 los bloques enteros
    if(bloquesNecesarios > 1) {
        memset(buf, 255, BLOCKSIZE);
        for(int i = 1; i < bloquesNecesarios; i++) {
            if(bwrite(posBloque++, buf) == ERROR_EXIT) {
                fprintf(stderr, 
                        "[Error en initMB()]: No se ha podido escribir el bloque %d", 
                        i);
                return ERROR_EXIT;
            }
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
    if(bwrite(posBloque, buf) == ERROR_EXIT) {
        fprintf(stderr, 
                "[Error en initMB()]: No se ha podido escribir el buffer en el bloque %d", 
                posBloque);
        return ERROR_EXIT;
    }

    // Ya que hemos marcado en el mapa de bits los bloques
    // ya ocupados, tenemos que actualizar la cantidad 
    // de bloques libres en el superbloque
    SB.cantBloquesLibres = SB.cantBloquesLibres - bloquesMetadatos;
    if(bwrite(posSB, &SB) == ERROR_EXIT) {
        fprintf(stderr, 
                "[Error en initMB()]: No se ha podido escribir el superbloque");
        return ERROR_EXIT;
    }

    return SUCCESS_EXIT;
}

/**
 * Inicializa todos los inodos del dispositivo
 * 
 * @returns         ERROR_EXIT si ha habido algun error,
 *                  SUCCESS_EXIT de lo contrario
 */
int initAI() {
    struct superbloque SB;
    struct inodo inodos[BLOCKSIZE/INODOSIZE];

    // Leemos el superbloque
    if(bread(posSB, &SB) == ERROR_EXIT) {
        fprintf(stderr, "[Error en initAI()]: No se ha podido leer el superbloque");
        return ERROR_EXIT;
    }

    int contInodos = SB.posPrimerInodoLibre + 1;

    // Recorremos todos los bloques de inodos
    for(int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++) {

        // Por cada inodo del bloque, lo inicializamos
        // como libre y lo enlazamos con el siguiente
        for(int j = 0; j < BLOCKSIZE / INODOSIZE; j++) {
            inodos[j].tipo = 'l';
            if(contInodos < SB.totInodos) {
                inodos[j].punterosDirectos[0] = contInodos;
                contInodos++;
            } else {
                inodos[j].punterosDirectos[0] = UINT_MAX;
                break;
            }

            // fprintf(stderr, "Escribiendo nodo %d del bloque %d con punterosDirectos[0] = %d\n", j, i, inodos[j].punterosDirectos[0]);
        }

        // Escribimos las modificaciones en el
        // dispositivo virtual
        if(bwrite(i, inodos) == ERROR_EXIT) {
            fprintf(stderr, "[Error en initAI()]: No se ha podido escribir el bloque de inodos %d", i);
            return ERROR_EXIT;
        }
    }


    return SUCCESS_EXIT;
}

/**
 * Escribe un valor determinado dado por parámetro
 * en un bit del mapa de bits
 * 
 * @param   nbloque numero de bloque a cambiar su valor en el MB
 * @param   bit     valor a actualizar el bloque
 * @return          devuelve ERROR_EXIT si da error, o SUCCESS_EXIT
 *                  si se ha escrito de forma correcta
 */
int escribir_bit(unsigned int nbloque, unsigned int bit){
    //declaramos SB y leemos los valores del mismo
    struct superbloque SB;

    if(bread(posSB, &SB) == ERROR_EXIT){
        return ERROR_EXIT;
    }
    //en que bloque del MB estamos (ABSOLUTO)
    int nbloqueMB = nbloque/BLOCKSIZE;
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;
    //en que byte del bloque estamos y lo mismo con el bit
    int posByte = nbloque/8;
    posByte = posByte % BLOCKSIZE;
    int posbit = nbloque%8;

    //buffer donde leeremos el bloque del mapa de bits
    unsigned char bufferMB[BLOCKSIZE];
    bread(nbloqueabs, bufferMB);
    //preparamos el byte a poner en uno
    unsigned char mascara = 128;
    mascara >>= posbit;

    //en funcion del caso marcamos como escrito
    //o como borrado
    if(bit == 1){
        bufferMB[posByte] |= mascara;
    } else {
        bufferMB[posByte] &= ~mascara;
    }

    if(bwrite(nbloqueabs,bufferMB) == ERROR_EXIT){
        return ERROR_EXIT;
    } else {
        return SUCCESS_EXIT;
    }
}
/**
 * Lee el valor que representa un bloque en nuestro MB
 * 
 * @param   nbloque bloque a leer en el MB
 * @returns         devuelve ERROR_EXIT si ha habido un error
 *                  en el proceso de lectura, o el valor en el que se encuentra el estado
 *                  de nuestro bloque en el MB
 */
char leer_bit(unsigned int nbloque){
    //declaramos SB y leemos los valores del mismo
    struct superbloque SB;

    if(bread(posSB, &SB) == ERROR_EXIT){
        return ERROR_EXIT;
    }
    //en que bloque del MB estamos (ABSOLUTO)
    int nbloqueMB = nbloque/BLOCKSIZE;
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;
    //en que byte del bloque estamos y lo mismo con el bit
    int posByte = nbloque/8;
    posByte = posByte % BLOCKSIZE;
    int posbit = nbloque%8;

    //buffer donde leeremos el bloque del mapa de bits
    unsigned char bufferMB[BLOCKSIZE];
    if(bread(nbloqueabs, bufferMB) == ERROR_EXIT){
        return ERROR_EXIT;
    }

    unsigned char mascara = 128; 
    mascara >>= posbit;          
    mascara &= bufferMB[posByte];
    mascara >>= (7 - posbit);

    return mascara;  
}

/**
 * Reservamos el primer bloque libre de nuestro MB para
 * 
 * @returns devuelve ERROR_EXIT si ha habido un error en su ejecución
 *          o el numero del primer bloque libre en nuestro sistema
 */
int reservar_bloque(){
    //reservamos memoria para el superbloque y lo leemos
    struct superbloque SB;

    if(bread(posSB, &SB) == ERROR_EXIT){
        return ERROR_EXIT;
    }
    //miramos si queda algun bloque libre
    if(SB.cantBloquesLibres == 0){
        printf("No quedan bloques libres");
        return ERROR_EXIT;
    }

    unsigned char buffer[BLOCKSIZE];
    unsigned char aux[BLOCKSIZE];
    unsigned char mascara = 128;
    memset(aux,255,BLOCKSIZE);
    //conseguimos el primer bloque de MB
    int nbloqueabs = SB.posPrimerBloqueMB;
    //y los bloques a recorrer, lo hacemos asi para ahorrar
    //uso de la cpu invocando a tamMB()
    int tamMB = SB.posPrimerBloqueAI - SB.posPrimerBloqueMB;

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
                    nbloque = ((nbloqueabs - SB.posPrimerBloqueMB)*BLOCKSIZE+posByte)*8 + posbit;
                    if(escribir_bit(nbloque,1) == ERROR_EXIT){
                        return ERROR_EXIT;
                    }
                    SB.cantBloquesLibres--;
                    //reseteamos el bloque a reservar por si había datos
                    memset(aux,0,BLOCKSIZE);
                    if(bwrite(nbloque,aux) == ERROR_EXIT){
                        bwrite(posSB, &SB);
                        return ERROR_EXIT;
                    }
                    bwrite(posSB, &SB);
                    return nbloque;
                }
            }
        }
    }
    //si ha llegado hasta aqui es porque no hay bloques libres
    //y por ende no se puede reservar ningún bloque
    return ERROR_EXIT;
}

/**
 * @brief libera un bloque poniendo un 0 en su correspondiente
 * bit del MB
 * 
 * @param nbloque numero de bloque a liberar
 * @return int numero de bloque liberado
 */
int liberar_bloque(unsigned int nbloque){
    struct superbloque SB;

    //cogemos los datos del superbloque y liberamos
    //el bloque en el MB
    if(bread(posSB, &SB) == ERROR_EXIT){
        return ERROR_EXIT;
    }

    escribir_bit(nbloque,0);

    //aumentamos el numero de bloques libres en el SB
    SB.cantBloquesLibres++;
    bwrite(posSB, &SB);

    return nbloque;
}

int escribir_inodo(unsigned int ninodo, struct inodo* inodo) {
    struct superbloque SB;
    struct inodo inodos[BLOCKSIZE/INODOSIZE];
    int bloqueRelativoInodo;

    if(bread(posSB, &SB) == ERROR_EXIT) {
        fprintf(stderr, "[Error en escribir_inodo]: No se ha podido leer el superbloque");
        return ERROR_EXIT;
    }

    // Calculamos el número relativo de bloque en
    // el que se encuentra el inodo solicitado
    bloqueRelativoInodo = ninodo/(BLOCKSIZE/INODOSIZE);
    if(ninodo%(BLOCKSIZE/INODOSIZE) != 0) bloqueRelativoInodo++;

    // Leemos el bloque solicitado
    if(bread(SB.posPrimerBloqueAI + bloqueRelativoInodo, inodos) == ERROR_EXIT) {
        fprintf(stderr, "[Error en escribir_inodo()]: No se ha podido leer el bloque de inodos");
        return ERROR_EXIT;
    }

    // Escribimos el inodo en el lugar correspondiente del array
    inodos[ninodo%(BLOCKSIZE/INODOSIZE)] = *inodo;

    if(bwrite(SB.posPrimerBloqueAI + bloqueRelativoInodo, inodos) == ERROR_EXIT) {
        fprintf(stderr, "[Error en escribir_inodo()]: No se ha podido escribir el bloque del inodo %d", ninodo);
        return ERROR_EXIT;
    }

    return SUCCESS_EXIT;
}

int leer_inodo(unsigned int ninodo, struct inodo *inodo) {
    struct superbloque SB;
    struct inodo inodos[BLOCKSIZE/INODOSIZE];
    int bloqueRelativoInodo;

    if(bread(posSB, &SB) == ERROR_EXIT) {
        fprintf(stderr, "[Error en leer_inodo]: No se ha podido leer el superbloque");
        return ERROR_EXIT;
    }

    // Calculamos el número relativo de bloque en
    // el que se encuentra el inodo solicitado
    bloqueRelativoInodo = ninodo/(BLOCKSIZE/INODOSIZE);
    if(ninodo%(BLOCKSIZE/INODOSIZE) != 0) bloqueRelativoInodo++;

    // Leemos el bloque solicitado
    if(bread(SB.posPrimerBloqueAI + bloqueRelativoInodo, inodos) == ERROR_EXIT) {
        fprintf(stderr, "Error leyendo bloque en leer_inodo()");
        return ERROR_EXIT;
    }

    *inodo = inodos[ninodo%(BLOCKSIZE/INODOSIZE)];

    return SUCCESS_EXIT;
}

int reservar_inodo(unsigned char tipo, unsigned char permisos) {
    struct superbloque SB;
    struct inodo* nodo;
    unsigned int posInodoReservado;

    nodo = malloc(sizeof(struct inodo));

    if(bread(posSB, &SB) == ERROR_EXIT) {
        fprintf(stderr, "[Error en reservar_inodo]: No se ha podido leer el superbloque");
        free(nodo);
        return ERROR_EXIT;
    }

    // Si no quedan inodos libres, no podemos reservar nada
    if(SB.cantInodosLibres == 0) {
        fprintf(stderr, "[Error en reservar_inodo()]: no quedan inodos libres");
        free(nodo);
        return ERROR_EXIT;
    }

    // Actualizamos la lista enlazada de inodos libres 
    posInodoReservado = SB.posPrimerInodoLibre;
    leer_inodo(posInodoReservado, nodo);
    SB.posPrimerInodoLibre = nodo->punterosDirectos[0];

    // Inicializamos los campos del inodo
    nodo->tipo = tipo;
    nodo->permisos = permisos;
    nodo->nlinks = 1;
    nodo->tamEnBytesLog = 0;
    nodo->atime = time(NULL);
    nodo->mtime = time(NULL);
    nodo->ctime = time(NULL);
    for(int i = 0; i < 12; i++) nodo->punterosDirectos[i] = 0;
    for(int i = 0; i < 3; i++) nodo->punterosIndirectos[i] = 0;

    escribir_inodo(posInodoReservado, nodo);

    SB.cantInodosLibres--;
    bwrite(posSB, &SB);

    free(nodo);
    return posInodoReservado;
}
