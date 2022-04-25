#include "bloques.h"

static int fd;

/**
 * Inicializa el flujo de datos, nuestro disco
 * 
 * @param   camino  ruta donde establecer el fd
 * @returns     SUCCESS_EXIT si ha funcionado bien
 *              ERROR_EXIT si ha habido un error
*/
int bmount(const char *camino) {
    umask(000);
    fd = open(camino, O_RDWR | O_CREAT, 0666);
    if(fd == -1) {
        fprintf(stderr, "Error %d montando el fichero: %s\n", errno, strerror(errno));
        return ERROR_EXIT;
    }

    return SUCCESS_EXIT;
}

/**
 * Cierra el descriptor de fichero, y por tanto nuestro
 * sistema de ficheros
 * 
 * @returns     SUCCESS_EXIT si ha funcionado bien
 *              ERROR_EXIT si ha habido un error     
*/
int bumount(){
    int error;

    error = close(fd);
    if(error == -1){
        fprintf(stderr,"Error %d desmontando el sistema de ficeros: %s\n", errno, strerror(errno));
        return ERROR_EXIT;
    }

    return SUCCESS_EXIT;
}
/**
 * Escribe un contenido en un bloque elegido
 * 
 * @param   nbloque numero de bloque a escribir
 * @param   buf contenido a escribir en el bloque
 * @returns     SUCCESS_EXIT si ha funcionado bien
 *              ERROR_EXIT si ha habido un error 
*/
int bwrite(unsigned int nbloque, const void *buf) {

    if(lseek(fd, nbloque * BLOCKSIZE, SEEK_SET) == -1) {
        fprintf(stderr, "Error %d moviendo el puntero para escribir el bloque %d: %s\n", errno, nbloque, strerror(errno));
        return ERROR_EXIT;
    }

    if(write(fd, buf, BLOCKSIZE) == -1) {
        fprintf(stderr, "Error %d escribiendo bloque: %s\n", errno, strerror(errno));
        return ERROR_EXIT;
    }

    return SUCCESS_EXIT;

}
/**
 * Lee el contenido de un bloque
 * 
 * @param   nbloque bloque elegido para leer
 * @param   buf buffer donde moveremos el contenido leido
 * @returns     SUCCESS_EXIT si ha funcionado bien
 *              ERROR_EXIT si ha habido un error 
*/
int bread(unsigned int nbloque, void *buf) {

    if(lseek(fd, nbloque * BLOCKSIZE, SEEK_SET) == -1) {
        fprintf(stderr, "Error %d moviendo el puntero para leer el bloque %d: %s\n", errno, nbloque, strerror(errno));
        return ERROR_EXIT;
    }

    if(read(fd, buf, BLOCKSIZE) == -1) {
        fprintf(stderr, "Error %d leyendo el bloque: %s\n", errno, strerror(errno));
        return ERROR_EXIT;
    }

    return SUCCESS_EXIT;
}
