#include "ficheros.h"

/**
 * Escribe el contenido de un buffer de memoria en un 
 * fichero/directorio
 * 
 * @param   ninodo          Número del inodo a escribir
 * @param   buf_original    Buffer con el contenido a escribir
 * @param   offset          Posición de escritura inicial en bytes lógicos, 
 *                          respecto al inodo
 * @param   nbytes          Número de bytes a escribir
 * @returns                 Número de bytes escritos si todo ha salido bien,
 *                          ERROR_EXIT de lo contrario
 */
int mi_write_f(unsigned int ninodo, const void* buf_original, unsigned int offset, unsigned int nbytes) {
    struct inodo inodo;
    unsigned char buf_bloque[BLOCKSIZE];
    int primerBL;
    int ultimoBL;
    int desp1;
    int desp2;
    int nbfisico;
    int bytesEscritos;

    if(leer_inodo(ninodo, &inodo) == ERROR_EXIT) {
        fprintf(stderr, "[Error en mi_write_f()]: error leyendo el inodo");
        #if DEBUG
            fprintf(stderr, "%s[ERROR EN LA LÍNEA %d DE FICHEROS.C]%s", RED, __LINE__, RESET_COLOR);
        #endif
        return ERROR_EXIT;
    }

    // Comprobamos que hay permisos de escritura
    if((inodo.permisos & 2) != 2) {
        fprintf(stderr, "[ERROR]: no hay permisos de escritura del fichero");
        #if DEBUG
            fprintf(stderr, "%s<ERROR EN LA LÍNEA %d DE FICHEROS.C>%s", RED, __LINE__, RESET_COLOR);
        #endif
        return ERROR_EXIT;
    }

    // Qué bloque/bloques lógicos hay que escribir
    primerBL = offset/BLOCKSIZE;
    ultimoBL = (offset + nbytes - 1)/BLOCKSIZE;

    // Desplazamiento de los bloques calculados donde cae el
    // offset
    desp1 = offset % BLOCKSIZE;
    desp2 = (offset + nbytes - 1)%BLOCKSIZE;

    nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1);

    if(primerBL == ultimoBL) {  // La operación afecta a un único bloque
        #if DEBUG
            fprintf(stderr, "[mi_write_f() -> Escribiendo en el bloque físico %d correspondiente al bloque lógico %d\n",
                    nbfisico,
                    primerBL);
        #endif
        if(bread(nbfisico, buf_bloque) == ERROR_EXIT) {
            fprintf(stderr, "[Error en mi_write_f()]: no se ha podido leer el bloque físico del inodo\n");
            #if DEBUG
                fprintf(stderr, "%s<ERROR EN LA LÍNEA %d DE FICHEROS.C>%s", RED, __LINE__, RESET_COLOR);
            #endif
            return ERROR_EXIT;
        }

        memcpy(buf_bloque + desp1, buf_original, nbytes);
        if(bwrite(nbfisico, buf_bloque) == ERROR_EXIT) {
            fprintf(stderr, "[Error en mi_write_f()]: no se ha podido escribir el bloque físico del inodo\n");
            #if DEBUG
                fprintf(stderr, "%s<ERROR EN LA LÍNEA %d DE FICHEROS.C>%s", RED, __LINE__, RESET_COLOR);
            #endif
            return ERROR_EXIT;
        }

        // Solo escribimos un bloque, lo cual equivale 
        // a BLOCKSIZE bytes
        bytesEscritos = BLOCKSIZE;
    } else {    // La operación afecta a más de un bloque

        bytesEscritos = 0;

        // Escritura del primer bloque
        if(bread(nbfisico, buf_bloque) == ERROR_EXIT) {
            fprintf(stderr, "[Error en mi_write_f()]: no se ha podido leer el bloque físico del inodo\n");
            #if DEBUG
                fprintf(stderr, "%s<ERROR EN LA LÍNEA %d DE FICHEROS.C>%s", RED, __LINE__, RESET_COLOR);
            #endif
            return ERROR_EXIT;
        }

        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);
        bytesEscritos += BLOCKSIZE - desp1;

        if(bwrite(nbfisico, buf_bloque) == ERROR_EXIT) {
            fprintf(stderr, "[Error en mi_write_f()]: no se ha podido escribir el bloque físico del inodo\n");
            #if DEBUG
                fprintf(stderr, "%s<ERROR EN LA LÍNEA %d DE FICHEROS.C>%s", RED, __LINE__, RESET_COLOR);
            #endif
            return ERROR_EXIT;
        }

        // Escritura de los bloques intermedios
        for(int i = primerBL + 1; i < ultimoBL; i++) {
            #if DEBUG
                fprintf(stderr, "%s<ESCRIBIENDO BLOQUE %d%s", YELLOW, i, RESET_COLOR);
            #endif

            nbfisico = traducir_bloque_inodo(ninodo, i, 1);

            if(bwrite(nbfisico, buf_original + (BLOCKSIZE - desp1) + (i - primerBL) * BLOCKSIZE) == ERROR_EXIT) {
                fprintf(stderr, "[Error en mi_write_f()]: no se ha podido escribir el bloque físico %d del inodo\n", i);
                #if DEBUG
                    fprintf(stderr, "%s<ERROR EN LA LÍNEA %d DE FICHEROS.C>%s", RED, __LINE__, RESET_COLOR);
                #endif
                return ERROR_EXIT;
            }

            // Estamos escribiendo bloques enteros,
            // por tanto, por cada iteración sumamos
            // el tamanyo de un bloque completo
            bytesEscritos += BLOCKSIZE;
        }

        // Ultimo bloque lógico
        nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 1);

        if(bread(nbfisico, buf_bloque) == ERROR_EXIT) {
            fprintf(stderr, "[Error en mi_write_f()]: no se ha podido leer el bloque físico %d del inodo\n", nbfisico);
            #if DEBUG
                fprintf(stderr, "%s<ERROR EN LA LÍNEA %d DE FICHEROS.C>%s", RED, __LINE__, RESET_COLOR);
            #endif
            return ERROR_EXIT;
        }

        memcpy(buf_bloque, buf_original + (nbytes - desp2 - 1), desp2 + 1);

        bytesEscritos += desp2 + 1;

        if(bwrite(nbfisico, buf_bloque) == ERROR_EXIT) {
            fprintf(stderr, "[Error en mi_write_f()]: no se ha podido escribir el bloque físico %d del inodo\n", nbfisico);
            #if DEBUG
                fprintf(stderr, "%s<ERROR EN LA LÍNEA %d DE FICHEROS.C>%s", RED, __LINE__, RESET_COLOR);
            #endif
            return ERROR_EXIT;
        }
    }

    // Actualizamos la metainformación del inodo
    leer_inodo(ninodo, &inodo);

    if(offset + nbytes >= inodo.tamEnBytesLog) {
        inodo.tamEnBytesLog = offset + nbytes;
        inodo.mtime = time(NULL);
    }

    escribir_inodo(ninodo, &inodo);

    return bytesEscritos; 
}

/**
 * Lee la información de un fichero/directorio y la almacena
 * en un buffer de memoria 
 * 
 * @param   ninodo          Posición del inodo a consultar
 * @param   buf_original    Buffer donde volcar la información
 * @param   offset          Posición de lectura inicial con respecto al inodo
 * @param   nbytes          Número de bytes a leer
 * 
 * @returns ERROR_EXIT si ha salido algo mal, el número de bytes
 *          leidos de lo contrario
 */
int mi_read_f(unsigned int ninodo, void* buf_original, unsigned int offset, unsigned int nbytes) {
    struct inodo inodo;
    unsigned char buf_bloque[BLOCKSIZE];
    int primerBL;
    int ultimoBL;
    int desp1;
    int desp2;
    int nbfisico;
    int bytesLeidos;

    if(leer_inodo(ninodo, &inodo) == ERROR_EXIT) {
        fprintf(stderr, "[Error en mi_read_f()]: no se ha podido leer el inodo %d", ninodo);
        #if DEBUG
            fprintf(stderr, "%s<ERROR EN LA LÍNEA %d DE FICHEROS.C>%s", RED, __LINE__, RESET_COLOR);
        #endif
        return ERROR_EXIT;
    }
    
    if((inodo.permisos & 4) != 4) {
        fprintf(stderr, "[ERROR]: no hay permisos de lectura del fichero");
        #if DEBUG
            fprintf(stderr, "%s<ERROR EN LA LÍNEA %d DE FICHEROS.C>%s", RED, __LINE__, RESET_COLOR);
        #endif
        return ERROR_EXIT;
    }

    // No se puede leer más alla del tamanyo
    // lógico del fichero. Por tanto, no leemos
    // nada y devolvemos 0
    if(offset >= inodo.tamEnBytesLog) return 0;

    if((offset + nbytes) >= inodo.tamEnBytesLog) {
        nbytes = inodo.tamEnBytesLog - offset;
    }

    primerBL = offset / BLOCKSIZE;
    ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;

    desp1 = offset % BLOCKSIZE;
    desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    nbfisico = traducir_bloque_inodo(ninodo, primerBL, 0);

    if(primerBL == ultimoBL) {  // Leemos solo un bloque
        if(nbfisico != ERROR_EXIT) {
            if(bread(nbfisico, buf_bloque) == ERROR_EXIT) {
                fprintf(stderr, "[Error en mi_read_f()]: no se ha podido leer el bloque físico del inodo\n");
                #if DEBUG
                    fprintf(stderr, "%s<ERROR EN LA LÍNEA %d DE FICHEROS.C>%s", RED, __LINE__, RESET_COLOR);
                #endif
                return ERROR_EXIT;
            }

            memcpy(buf_bloque + desp1, buf_original, nbytes);
        }

        bytesLeidos = nbytes;
    } else { // Leemos más de un bloque
        bytesLeidos = 0;

        // Lectura del primer bloque
        if(nbfisico != ERROR_EXIT) {
            if(bread(nbfisico, buf_bloque) == ERROR_EXIT) {
                fprintf(stderr, "[Error en mi_read_f()]: no se ha podido leer el bloque físico del inodo\n");
                #if DEBUG
                    fprintf(stderr, "%s<ERROR EN LA LÍNEA %d DE FICHEROS.C>%s", RED, __LINE__, RESET_COLOR);
                #endif
                return ERROR_EXIT;
            }

            memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);
        }

        bytesLeidos += BLOCKSIZE - desp1;

        // Lectura de los bloques intermedios
        for(int i = primerBL + 1; i < ultimoBL; i++) {
            #if DEBUG
                fprintf(stderr, "%s<LEYENDO BLOQUE %d%s", YELLOW, i, RESET_COLOR);
            #endif

            nbfisico = traducir_bloque_inodo(ninodo, i, 0);

            if(nbfisico != ERROR_EXIT) {
                if(bread(nbfisico, buf_bloque + (BLOCKSIZE - desp1) + (i - primerBL) * BLOCKSIZE) == ERROR_EXIT) {
                    fprintf(stderr, "[Error en mi_write_f()]: no se ha podido leer el bloque físico del inodo\n");
                    #if DEBUG
                        fprintf(stderr, "%s<ERROR EN LA LÍNEA %d DE FICHEROS.C>%s", RED, __LINE__, RESET_COLOR);
                    #endif
                    return ERROR_EXIT;
                }

                memcpy(buf_bloque + (BLOCKSIZE - desp1) + (i - primerBL) * BLOCKSIZE, buf_original, BLOCKSIZE - desp1);
            }

            bytesLeidos += BLOCKSIZE;
        }
        
        // Ultimo bloque lógico
        nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 1);

        if(nbfisico != ERROR_EXIT) {
            if(bread(nbfisico, buf_bloque) == ERROR_EXIT) {
                fprintf(stderr, "[Error en mi_read_f()]: no se ha podido leer el bloque físico %d del inodo\n", nbfisico);
                #if DEBUG
                    fprintf(stderr, "%s<ERROR EN LA LÍNEA %d DE FICHEROS.C>%s", RED, __LINE__, RESET_COLOR);
                #endif
                return ERROR_EXIT;
            }
            memcpy(buf_bloque, buf_original + (nbytes - desp2 - 1), desp2 + 1);
        }


        bytesLeidos += desp2 + 1;
    }

    if(leer_inodo(ninodo, &inodo) == ERROR_EXIT) {
        fprintf(stderr, "[Error en mi_read_f()]: no se ha podido leer el inodo %d\n", ninodo);
        #if DEBUG
            fprintf(stderr, "%s<ERROR EN LA LÍNEA %d DE FICHEROS.C>%s", RED, __LINE__, RESET_COLOR);
        #endif
        return ERROR_EXIT;
    }

    inodo.atime = time(NULL);

    if(escribir_inodo(ninodo, &inodo) == ERROR_EXIT) {
        fprintf(stderr, "[Error en mi_read_f()]: no se ha podido escribir el inodo %d\n", ninodo);
        #if DEBUG
            fprintf(stderr, "%s<ERROR EN LA LÍNEA %d DE FICHEROS.C>%s", RED, __LINE__, RESET_COLOR);
        #endif
        return ERROR_EXIT;
    }

    return bytesLeidos;
}

/**
 * Vuelca en un struct STAT la metainformación de un fichero/directorio
 * correspondiente a un ninodo
 * 
 * @param   ninodo  Posición del inodo a consultar
 * @param   p_stat  Puntero al struct STAT donde volcar los metadatos  
 * 
 * @returns ERROR_EXIT si ha salido algo mal, SUCCESS_EXIT de lo contrario
 */
int mi_stat_f(unsigned int ninodo, struct STAT* p_stat) {
    struct inodo inodo;    

    if(leer_inodo(ninodo, &inodo) == ERROR_EXIT) {
        fprintf(stderr, "[Error en mi_stat_f()]: no se ha podido leer el inodo %d\n", ninodo);
        #if DEBUG
            fprintf(stderr, "%s<ERROR EN LA LÍNEA %d DE FICHEROS.C>%s", RED, __LINE__, RESET_COLOR);
        #endif
        return ERROR_EXIT;
    }

    p_stat->tipo = inodo.tipo;
    p_stat->permisos = inodo.permisos;
    p_stat->atime = inodo.atime;
    p_stat->ctime = inodo.ctime;
    p_stat->mtime = inodo.mtime;
    p_stat->nlinks = inodo.nlinks;
    p_stat->numBloquesOcupados = inodo.numBloquesOcupados;
    p_stat->tamEnBytesLog = inodo.tamEnBytesLog;

    return SUCCESS_EXIT;
}

/**
 * Cambia los permisos de un inodo determinado
 * 
 * @param   ninodo      Posición del inodo a modificar
 * @param   permisos    Nuevos permisos que queremos que tenga el inodo
 * 
 * @returns ERROR_EXIT si ha salido algo mal, SUCCESS_EXIT de lo contrario
 */
int mi_chmod_f(unsigned int ninodo, unsigned char permisos) {
    struct inodo inodo;

    if(leer_inodo(ninodo, &inodo) == ERROR_EXIT) {
        fprintf(stderr, "[Error en mi_chmod_f()]: no se ha podido leer el inodo %d\n", ninodo);
        #if DEBUG
            fprintf(stderr, "%s<ERROR EN LA LÍNEA %d DE FICHEROS.C>%s", RED, __LINE__, RESET_COLOR);
        #endif
        return ERROR_EXIT;
    }

    inodo.permisos = permisos;

    if(escribir_inodo(ninodo, &inodo) == ERROR_EXIT) {
        fprintf(stderr, "[Error en mi_chmod_f()]: no se ha podido escribir el inodo %d\n", ninodo);
        #if DEBUG
            fprintf(stderr, "%s<ERROR EN LA LÍNEA %d DE FICHEROS.C>%s", RED, __LINE__, RESET_COLOR);
        #endif
        return ERROR_EXIT;
    }

    return SUCCESS_EXIT;
}

/**
 * @brief se trunca un fichero con una largaria y inodo determinados
 * 
 * @param ninodo el inodo correspondiente al fichero
 * @param nbytes la largaria del fichero
 * @return int el número de bloques liberados
 */
int mi_truncar_f(unsigned int ninodo, unsigned int nbytes){
    struct inodo inodo;
    int primerBL = 0;
    int bloquesLiberados = 0;

    if(leer_inodo(ninodo, &inodo) == ERROR_EXIT) {
        fprintf(stderr, "[Error en mi_truncar_f()]: no se ha podido leer el inodo %d\n", ninodo);
        #if DEBUG
            fprintf(stderr, "%s<ERROR EN LA LÍNEA %d DE FICHEROS.C>%s", RED, __LINE__, RESET_COLOR);
        #endif
        return ERROR_EXIT;
    }
    //TODO MIRAR SI TIENE PERMISOS DE ESCRITURA
    if(nbytes % BLOCKSIZE == 0){
        primerBL = nbytes/BLOCKSIZE;
    } else {
        primerBL = nbytes / BLOCKSIZE + 1;
    }
    //que libere los bloques logicos de nuestro inodo desde el primero
    bloquesLiberados = liberar_bloques_inodo(primerBL,&inodo);
    //tras esto modificamos nuestros atributos del inodo
    inodo.mtime = time(NULL);
    inodo.ctime = time(NULL);
    inodo.tamEnBytesLog = nbytes;
    inodo.numBloquesOcupados = inodo.numBloquesOcupados - bloquesLiberados;

    return bloquesLiberados;
}