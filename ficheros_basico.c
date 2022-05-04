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
    int posBloque = posSB + 1;

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
        int valorBitsSueltos = 0;
        for(int i = 7; bitsSueltos > 0; i--) {
            valorBitsSueltos += (int) pow(2, i);
            bitsSueltos--;
        }
        buf[bytesNecesarios] = valorBitsSueltos;
    }

    // Rellenamos los bytes que sobren a 0
    for(int i = bytesNecesarios + 1; i < BLOCKSIZE; i++) {
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
    unsigned char bufferMB[BLOCKSIZE];
    unsigned char mascara;

    if(bread(posSB, &SB) == ERROR_EXIT){
        fprintf(stderr, "[Error en escribir_bit()]: No se ha podido leer el superbloque");
        return ERROR_EXIT;
    }

    //en que byte del bloque estamos y lo mismo con el bit
    int posByte = nbloque/8;
    int posbit = nbloque % 8;
    //en que bloque del MB estamos (ABSOLUTO)
    int nbloqueMB = posByte/BLOCKSIZE;
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;

    //buffer donde leeremos el bloque del mapa de bits
    if(bread(nbloqueabs, bufferMB) == ERROR_EXIT) {
        fprintf(stderr, "[Error en escribir_bit()]: No se ha podido leer el bloque");
        return ERROR_EXIT;
    };

    posByte = posByte % BLOCKSIZE;

    //preparamos el byte a poner en uno
    mascara = 128;
    mascara >>= posbit;

    //en funcion del caso marcamos como escrito
    //o como borrado
    if(bit == 1){
        bufferMB[posByte] |= mascara;
    } else {
        bufferMB[posByte] &= ~mascara;
    }

    if(bwrite(nbloqueabs,bufferMB) == ERROR_EXIT){
        fprintf(stderr, "[Error en escribir_bit()]: No se ha podido escribir el bloque");
        return ERROR_EXIT;
    }

    return SUCCESS_EXIT;
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
    unsigned char bufferMB[BLOCKSIZE];
    unsigned char mascara;

    if(bread(posSB, &SB) == ERROR_EXIT){
        fprintf(stderr, "[Error en leer_bit()]: No se ha podido leer el superbloque");
        return ERROR_EXIT;
    }

    //en que byte del bloque estamos y lo mismo con el bit
    int posByte = nbloque/8;
    int posbit = nbloque%8;
    //en que bloque del MB estamos (ABSOLUTO)
    int nbloqueMB = posByte/BLOCKSIZE;
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;

    //buffer donde leeremos el bloque del mapa de bits
    if(bread(nbloqueabs, bufferMB) == ERROR_EXIT){
        fprintf(stderr, "[Error en leer_bit()]: No se ha podido leer el bloque");
        return ERROR_EXIT;
    }

    posByte = posByte % BLOCKSIZE;

    mascara = 128; 
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
    unsigned char bufferMB[BLOCKSIZE];
    unsigned char bufferAux[BLOCKSIZE];
    unsigned int nbloque;

    if(bread(posSB, &SB) == ERROR_EXIT){
        return ERROR_EXIT;
    }
    //miramos si queda algun bloque libre
    if(SB.cantBloquesLibres == 0){
        printf("No quedan bloques libres\n");
        return ERROR_EXIT;
    }

    memset(bufferAux,255,BLOCKSIZE);
    //conseguimos el primer bloque de MB
    int nbloqueabs = SB.posPrimerBloqueMB;

    //vamos comparando todos los blqoues a ver si hay algo
    //en alguno de ellos
    int posBloqueMB = SB.posPrimerBloqueMB;
    int posByte = 0;
    int posbit = 0;

    //leemos el primer bloque del mapa de bits
    if(bread(posBloqueMB,bufferMB)){
        printf("ERROR LEYENDO BLOQUE %d EN RESERVAR_BLOQUE()",posBloqueMB);
    }

    while(memcmp(bufferMB,bufferAux,BLOCKSIZE) == 0){
        if(bread(posBloqueMB,bufferMB)){
            printf("ERROR LEYENDO BLOQUE %d EN RESERVAR_BLOQUE()\n",posBloqueMB);
            return ERROR_EXIT;
        }
        
        if(memcmp(bufferMB,bufferAux,BLOCKSIZE) != 0){
            break;
        }

        posBloqueMB++;
    }
    

    //ahora dentro de ese bloque avanzamos de byte en byte hasta encontrar uno
    //con un bit vacio como minimo

    while(bufferMB[posByte] == 255){
        posByte++;
    }

    //finalmente nos falta determinar que bit dentro del byte es el que esta libre

    unsigned char mascara = 128;
    
    while(bufferMB[posByte] & mascara){
        //vamos corriendo bit a bit
        bufferMB[posByte] <<= 1;
        posbit++;
    }

    //tenemos todos los datos para determinar el bloque a reservar
    nbloque = ((posBloqueMB - SB.posPrimerBloqueMB)*BLOCKSIZE+posByte) * 8 + posbit;
    
    //marquemos este bloque como reservado en el MB y retornemoslo a la vez
    //actualizando los valores del SB y limpiando el bloque reservado

    if(escribir_bit(nbloque,1) == ERROR_EXIT){
        printf("ERROR ESCRIBIENDO EL BIT EN EL MB DEL BLOQUE %d EN RESERVAR_BLOQUE()\n",nbloque);
        return ERROR_EXIT;
    }

    SB.cantBloquesLibres--;

    memset(bufferAux,0,BLOCKSIZE);
    if(bwrite(SB.posUltimoBloqueDatos + nbloque - 1,bufferAux) == ERROR_EXIT){
        printf("ERROR ESCRIBIENDO EL BLOQUE %d EN RESERVAR_BLOQUE()\n",nbloque);
        return ERROR_EXIT;
    }

    if(bwrite(posSB,&SB) == ERROR_EXIT){
        printf("ERROR ESCRIBIENDO EL SUPERBLOQUE EN RESERVAR_BLOQUE()");
        return ERROR_EXIT;
    }

    return nbloque;


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
        fprintf(stderr, "[Error en escribir_inodo()]: No se ha podido escribir el bloque del inodo %d\n", ninodo);
        return ERROR_EXIT;
    }

    return SUCCESS_EXIT;
}

int leer_inodo(unsigned int ninodo, struct inodo *inodo) {
    struct superbloque SB;
    struct inodo inodos[BLOCKSIZE/INODOSIZE];
    int bloqueRelativoInodo;

    if(bread(posSB, &SB) == ERROR_EXIT) {
        fprintf(stderr, "[Error en leer_inodo]: No se ha podido leer el superbloque\n");
        return ERROR_EXIT;
    }

    // Calculamos el número relativo de bloque en
    // el que se encuentra el inodo solicitado
    bloqueRelativoInodo = ninodo/(BLOCKSIZE/INODOSIZE);
    if(ninodo%(BLOCKSIZE/INODOSIZE) != 0) bloqueRelativoInodo++;

    // Leemos el bloque solicitado
    if(bread(SB.posPrimerBloqueAI + bloqueRelativoInodo, inodos) == ERROR_EXIT) {
        fprintf(stderr, "Error leyendo bloque en leer_inodo()\n");
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
        fprintf(stderr, "[Error en reservar_inodo]: No se ha podido leer el superbloque\n");
        free(nodo);
        return ERROR_EXIT;
    }

    // Si no quedan inodos libres, no podemos reservar nada
    if(SB.cantInodosLibres == 0) {
        fprintf(stderr, "[Error en reservar_inodo()]: no quedan inodos libres\n");
        free(nodo);
        return ERROR_EXIT;
    }

    // Actualizamos la lista enlazada de inodos libres 
    posInodoReservado = SB.posPrimerInodoLibre;
    SB.posPrimerInodoLibre++;

    // Inicializamos los campos del inodo
    nodo->tipo = tipo;
    nodo->permisos = permisos;
    nodo->nlinks = 1;
    nodo->tamEnBytesLog = 0;
    nodo->numBloquesOcupados = 0;
    nodo->atime = time(NULL);
    nodo->mtime = time(NULL);
    nodo->ctime = time(NULL);
    for(int i = 0; i < 12; i++) nodo->punterosDirectos[i] = 0;
    for(int i = 0; i < 3; i++) nodo->punterosIndirectos[i] = 0;

    if(escribir_inodo(posInodoReservado, nodo) == ERROR_EXIT){
        printf("ERROR ESCRIBIENDO EL INODO %d EN RESERVAR_INODO()",posInodoReservado);
        return ERROR_EXIT;
    }

    SB.cantInodosLibres--;
    if(bwrite(posSB, &SB) == ERROR_EXIT){
        printf("ERROR ESCRIBIENDO EL SUPERBLOQUE EN RESERVAR_INODO()");
        return ERROR_EXIT;
    }

    free(nodo);
    return posInodoReservado;
}
/**
 * @brief funcion que nos dice a que rango de punteros se sitúa el
 * bloque lógico que buscamos
 * 
 * @param inodo
 * @param nblogico 
 * @param ptr 
 * @return bloque lógico en el que se encuentra 
 */
int obtener_nRangoBL(struct inodo* inodo, unsigned int nblogico, unsigned int* ptr){

    if(nblogico<DIRECTOS){
        *ptr = inodo->punterosDirectos[nblogico];
        return 0;
    }

    if(nblogico<INDIRECTOS0){
        *ptr = inodo->punterosIndirectos[0];
        return 1;
    }

    if(nblogico<INDIRECTOS1){
        *ptr = inodo->punterosIndirectos[1];
        return 2;
    }

    if(nblogico<INDIRECTOS2){
        *ptr = inodo->punterosIndirectos[2];
        return 3;
    }
    //Si llega hasta aquí es porque no habrá entrado en ningun
    //rango de inodos
    *ptr = 0;
    perror("Bloque lógico fuera de rango\n");
    return -1;
}

int obtener_indice(unsigned int nblogico, int nivel_punteros){
    //Cuando el inodo apunta a uno de los 12 primeros bloques
    if(nblogico<DIRECTOS){
        return nblogico;
    }
    else if(nblogico<INDIRECTOS0){
        return nblogico-DIRECTOS;
    }
    else if(nblogico<INDIRECTOS1){
        if(nivel_punteros == 2){
            return (nblogico - INDIRECTOS0) / NPUNTEROS;
        }
        if(nivel_punteros == 1){
            return (nblogico - INDIRECTOS0) % NPUNTEROS;
        }
    }
    else if(nblogico<INDIRECTOS2){
        if(nivel_punteros == 3){
            return (nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS);
        }
        else if(nivel_punteros == 2){
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) / NPUNTEROS;
        }
        else if(nivel_punteros == 1){
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) % NPUNTEROS;
        }
    } 
    
    // Si hemos llegado aquí es porque ha habido error
    fprintf(stderr, "[Error en obtener_indice()]: no se cumple ninguna condición, se devuelve -1\n");
    return -1;
}

/**
 * @brief obtener el bloque físico correspondiente de cierto inodo
 * 
 * @param ninodo numero de inodo a traducir
 * @param nblogico el numero de bloque lógico 
 * @param reservar si queremos o no reservar el inodo
 * @return int ERROR_EXIT en caso de que haya ido mal, 
 */
int traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, unsigned char reservar){
    struct inodo inodo;
    unsigned int ptr,ptr_ant = 0;
    int salvar_inodo,nRangoBL,nivel_punteros,indice = 0;
    int buffer[NPUNTEROS];

    leer_inodo(ninodo,&inodo);
    nRangoBL = obtener_nRangoBL(&inodo,nblogico,&ptr);
    nivel_punteros = nRangoBL;
    while(nivel_punteros>0){
        if(ptr == 0){
            if(reservar == 0) {
                return ERROR_EXIT;
            }
            else {
                salvar_inodo = 1;
                ptr = reservar_bloque();
                inodo.numBloquesOcupados++;
                inodo.ctime = time(NULL);
                if(nivel_punteros == nRangoBL){
                    inodo.punterosIndirectos[nRangoBL - 1] = ptr;
                    
                        fprintf(stderr, "[traducir_bloque_inodo() -> inodo.punterosIndirectos[%d - 1] = %d\n", nRangoBL, ptr);
                    
                }
                else {
                    buffer[indice] = ptr;
                    
                        fprintf(stderr, "[traducir_bloque_inodo() -> buffer[%d] = %d\n", indice , ptr);
                    
                    bwrite(ptr_ant,buffer);
                }
                memset(buffer,0,BLOCKSIZE);
            }
        } else {
            bread(ptr,buffer);
        }
        indice = obtener_indice(nblogico,nivel_punteros);
        ptr_ant = ptr;
        ptr = buffer[indice];
        nivel_punteros--;
    }

    if(ptr == 0){
        if(reservar == 0){
            return ERROR_EXIT;
        } else {
            salvar_inodo = 1;
            ptr = reservar_bloque();
            inodo.numBloquesOcupados++;
            inodo.ctime = time(NULL);
            if (nRangoBL == 0){
                inodo.punterosDirectos[nblogico] = ptr;
                
                    fprintf(stderr, "[traducir_bloque_inodo() -> inodo.punterosDirectos[%d] = %d\n", nblogico, ptr);
                
            } else {
                buffer[indice] = ptr;
                
                    fprintf(stderr, "[traducir_bloque_inodo() -> buffer[%d] = %d\n", indice , ptr);
                
                bwrite(ptr_ant,buffer);
            }
        }
    }

    if(salvar_inodo == 1){
        escribir_inodo(ninodo, &inodo); 
    }
    return ptr;
}

/**
 * @brief función que libera al completa un inodo
 * 
 * @param ninodo el número de inodo a liberar
 * @return int el número de bloques liberados del inodo
 */
int liberar_inodo(unsigned int ninodo){
    struct inodo inodo;
    struct superbloque SB;
    int bloquesLiberados = 0;
    //leemos el superbloque y el inodo a liberar
    if(bread(0,&SB) == ERROR_EXIT) {
        fprintf(stderr, "[Error en mi_write_f()]: no se ha podido leer el bloque físico del inodo\n");
        #if DEBUG
            fprintf(stderr, "%s<ERROR EN LA LÍNEA %d DE FICHEROS.C>%s", RED, __LINE__, RESET_COLOR);
        #endif
        return ERROR_EXIT;
    }
    
    if(leer_inodo(ninodo, &inodo) == ERROR_EXIT) {
    fprintf(stderr, "[Error en mi_chmod_f()]: no se ha podido leer el inodo %d\n", ninodo);
    #if DEBUG
        fprintf(stderr, "%s<ERROR EN LA LÍNEA %d DE FICHEROS.C>%s", RED, __LINE__, RESET_COLOR);
    #endif
    return ERROR_EXIT;
    }
    //liberamos los bloques a los que apunta el inodo
    bloquesLiberados = liberar_bloques_inodo(0,&inodo);
    inodo.numBloquesOcupados = inodo.numBloquesOcupados - bloquesLiberados;
    //si no se han eliminado todos los bloques ha habido algún error
    if(inodo.numBloquesOcupados != 0){
        printf("Error, aún hay bloques en el inodo sin liberar");
    }
    //Al haber liberado todos los bloques del inodo este será libre y no tendrá
    //ningun tamaño
    inodo.tipo = 'l';
    inodo.tamEnBytesLog = 0;
    //hacemos que el inodo apunte al primer inodo del superbloque, aumentamos la cadena
    inodo.punterosDirectos[0] = SB.posPrimerInodoLibre;
    SB.posPrimerInodoLibre = ninodo;
    SB.cantInodosLibres++;
    //no hace falta escribir ya que pasamos por referencia
    return ninodo;
}

/**
 * @brief libera todos los bloques de un inodo, función de apoyo de liberar inodo
 * 
 * @param primerBL el primer bloque lógico que presenta el inodo
 * @param inodo el inodo al que vamos a liberar sus correspondientes bloques
 * @return int devuelve el número de bloques liberados del inodo
 */
int liberar_bloques_inodo(unsigned int primerBL, struct inodo *inodo){
    unsigned int nivel_punteros,indice,ptr,nBL,ultimoBL = 0;
    int nRangoBL = 0;
    unsigned int bloques_punteros[3][NPUNTEROS];
    unsigned char bufAux_punteros[BLOCKSIZE];
    int ptr_nivel[3];
    int indices[3];
    int liberados = 0;

    //Si no hay nada en ese inodo no hay bloques a liberar
    if(inodo->tamEnBytesLog == 0){
        return EXIT_SUCCESS;
    }
    //Determinamos cuantos bloques tiene el inodo
    if(inodo->tamEnBytesLog % BLOCKSIZE == 0){
        ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE - 1;
    } else {
        ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE;
    }

    //Preparamos el buffer para liberar los bloques
    memset(bufAux_punteros,0,BLOCKSIZE);

    for(nBL = primerBL; nBL < ultimoBL; nBL++){
        nRangoBL = obtener_nRangoBL(inodo,nBL,&ptr);
        if(nRangoBL < 0){
            printf("Error al obtener el rango de bloque lógico");
        }
        nivel_punteros = nRangoBL;

        //aun hay bloques de punteros por leer
        while(ptr>0 && nivel_punteros>0){
            indice = obtener_indice(nBL, nivel_punteros);
            if(indice == 0 || nBL == primerBL){
                bread(ptr,bloques_punteros[nivel_punteros-1]);
            }
            ptr_nivel[nivel_punteros - 1] = ptr;
            indices[nivel_punteros - 1] = indice;
            ptr = bloques_punteros[nivel_punteros-1][indice];
            nivel_punteros--;
        }

        //Si existe bloque de datos
        if(ptr>0){
            liberar_bloque(ptr);
            liberados++;
        }
        //Puntero directo?
        if(nRangoBL == 0) {
            inodo->punterosDirectos[nBL] = 0;
        } else {
            nivel_punteros = 1;
            while (nivel_punteros <= nRangoBL)
            {
                indice = indices[nivel_punteros - 1];
                bloques_punteros[nivel_punteros - 1][indice] = 0;
                ptr = ptr_nivel[nivel_punteros - 1];
                if(memcmp(bloques_punteros[nivel_punteros - 1],bufAux_punteros, BLOCKSIZE) == 0){
                    //No quedan más bloques ocupados, liberar bloque punteros
                    liberar_bloque(ptr);
                    liberados++;
                    if(nivel_punteros == nRangoBL){
                        inodo->punterosIndirectos[nRangoBL - 1] = 0;
                    }
                    nivel_punteros++;
                } else {
                    bwrite(ptr,bloques_punteros[nivel_punteros - 1]);
                    nivel_punteros = nRangoBL + 1;
                }
            }
            
        }

    }
    return liberados;
}
