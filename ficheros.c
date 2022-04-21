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
            fprintf(stderr, "%s<ERROR EN LA LÍNEA %d DE FICHEROS.C>%s", RED, __LINE__, RESET_COLOR);
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

    if(offset >= inodo.tamEnBytesLog) {
        inodo.tamEnBytesLog += offset;
        inodo.mtime = time(NULL);
    }

    escribir_inodo(ninodo, &inodo);

    return bytesEscritos; 
}

int mi_read_f(unsigned int ninodo, void* buf_original, unsigned int offset, unsigned int nbytes) {
    struct inodo inodo;
    unsigned char buf_bloque[BLOCKSIZE];
    int primerBL;
    int ultimoBL;
    int desp1;
    int desp2;
    int nbfisico;
    int bytesLeidos;

    leer_inodo(ninodo, &inodo);
    
    if((inodo.permisos & 4) != 4) {
        fprintf(stderr, "[ERROR]: no hay permisos de escritura del fichero");
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
                fprintf(stderr, "[Error en mi_write_f()]: no se ha podido leer el bloque físico del inodo\n");
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
                if(bread(nbfisico, buf_bloque) == ERROR_EXIT) {
                    fprintf(stderr, "[Error en mi_write_f()]: no se ha podido leer el bloque físico del inodo\n");
                    #if DEBUG
                        fprintf(stderr, "%s<ERROR EN LA LÍNEA %d DE FICHEROS.C>%s", RED, __LINE__, RESET_COLOR);
                    #endif
                    return ERROR_EXIT;
                }

                memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);
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

int mi_chmod_f(unsigned int ninodo, unsigned char permisos) {

}