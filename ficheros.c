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

    if(leer_inodo(ninodo, &inodo) == ERROR_EXIT) {
        fprintf(stderr, "[Error en mi_write_f()]: error leyendo el inodo");
        return ERROR_EXIT;
    }

    // Comprobamos que hay permisos de escritura
    if((inodo.permisos & 2) != 2) {
        fprintf(stderr, "[ERROR]: no hay permisos de escritura del fichero");
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
                fprintf(stderr, "%s<ERROR EN LA LÍNEA 47 DE FICHEROS.C>%s", RED, RESET_COLOR);
            #endif
            return ERROR_EXIT;
        }

        memcpy(buf_bloque + desp1, buf_original, nbytes);
        if(bwrite(nbfisico, buf_bloque) == ERROR_EXIT) {
            fprintf(stderr, "[Error en mi_write_f()]: no se ha podido escribir el bloque físico del inodo\n");
            #if DEBUG
                fprintf(stderr, "%s<ERROR EN LA LÍNEA 56 DE FICHEROS.C>%s", RED, RESET_COLOR);
            #endif
            return ERROR_EXIT;
        }
    } else {    // La operación afecta a más de un bloque
        // Escritura del primer bloque
        if(bread(nbfisico, buf_bloque) == ERROR_EXIT) {
            fprintf(stderr, "[Error en mi_write_f()]: no se ha podido leer el bloque físico del inodo\n");
            #if DEBUG
                fprintf(stderr, "%s<ERROR EN LA LÍNEA 64 DE FICHEROS.C>%s", RED, RESET_COLOR);
            #endif
            return ERROR_EXIT;
        }

        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);

        if(bwrite(nbfisico, buf_bloque) == ERROR_EXIT) {
            fprintf(stderr, "[Error en mi_write_f()]: no se ha podido escribir el bloque físico del inodo\n");
            #if DEBUG
                fprintf(stderr, "%s<ERROR EN LA LÍNEA 74 DE FICHEROS.C>%s", RED, RESET_COLOR);
            #endif
            return ERROR_EXIT;
        }

        // Escritura de los bloques intermedios
        for(int i = primerBL + 1; i < ultimoBL; i++) {
            if(bwrite(nbfisico, buf_original + (BLOCKSIZE - desp1) + (i - primerBL) * BLOCKSIZE) == ERROR_EXIT) {
                fprintf(stderr, "[Error en mi_write_f()]: no se ha podido escribir el bloque físico %d del inodo\n", i);
                #if DEBUG
                    fprintf(stderr, "%s<ERROR EN LA LÍNEA 85 DE FICHEROS.C>%s", RED, RESET_COLOR);
                #endif
                return ERROR_EXIT;
            }
        }

        // Ultimo bloque lógico
        if(bread(nbfisico, buf_bloque) == ERROR_EXIT) {
            fprintf(stderr, "[Error en mi_write_f()]: no se ha podido leer el bloque físico %d del inodo\n", nbfisico);
            #if DEBUG
                fprintf(stderr, "%s<ERROR EN LA LÍNEA 95 DE FICHEROS.C>%s", RED, RESET_COLOR);
            #endif
            return ERROR_EXIT;
        }

        memcpy(buf_bloque, buf_original + (nbytes - desp2 - 1), desp2 + 1);

        if(bwrite(nbfisico, buf_bloque) == ERROR_EXIT) {
            fprintf(stderr, "[Error en mi_write_f()]: no se ha podido escribir el bloque físico %d del inodo\n", nbfisico);
            #if DEBUG
                fprintf(stderr, "%s<ERROR EN LA LÍNEA 105 DE FICHEROS.C>%s", RED, RESET_COLOR);
            #endif
            return ERROR_EXIT;
        }
    }

    // Actualizamos la metainformación del inodo
    leer_inodo(ninodo, &inodo);

    // TODO: HACER LO DE ACTUALIZAR tamEnBytesLog
    inodo.mtime = time(NULL);
    escribir_inodo(ninodo, &inodo);

    return nbytes; // TODO: no se si esto es asi, hay que revisarlo
}

int mi_read_f(unsigned int ninodo, void* buf_original, unsigned int offset, unsigned int nbytes) {

}