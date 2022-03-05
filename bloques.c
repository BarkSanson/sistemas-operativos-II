#include "bloques.h"

static int fd;

int bmount(const char *camino) {
    umask(000);
    fd = open(camino, O_RDWR | O_CREAT, 0666);
    if(fd == -1) {
        fprintf(stderr, "Error %d montando el fichero: %s", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int bumount(){
    int error;

    error = close(fd);
    if(error == -1){
        fprintf(stderr,"Error %d desmontando el sistema de ficeros: %s", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int bwrite(unsigned int nbloque, const void *buf) {

    if(lseek(fd, nbloque * BLOCKSIZE, SEEK_SET) == -1) {
        fprintf(stderr, "Error %d moviendo el puntero para escribir un bloque: %s", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    if(write(fd, buf, BLOCKSIZE) == -1) {
        fprintf(stderr, "Error %d escribiendo bloque: %s", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

}

int bread(unsigned int nbloque, void *buf) {

    if(lseek(fd, nbloque * BLOCKSIZE, SEEK_SET) == -1) {
        fprintf(stderr, "Error %d moviendo el puntero para leer un bloque: %s", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    if(read(fd, buf, BLOCKSIZE) == -1) {
        fprintf(stderr, "Error %d leyendo el bloque: %s", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
