#include "bloques.h";

static int fd;

int bmount(const char *camino) {
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
        fprinf(stderr,"Error %d desmontando el sistema de ficeros: %s", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int bwrite(unsigned int nbloque, const void *buf) {
    int error;

    error = lseek(fd, nbloque*BLOCKSIZE, SEEK_SET);
    if(error == -1) {
        fprintf(stderr, "Error %d escribiendo bloque: %s", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    error = write(fd, buf, BLOCKSIZE);
    if(error = -1) {
        fprintf(stderr, "Error %d escribiendo bloque: %s", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

}

int bread(unsigned int nbloque, void *buf) {
    int error;

    lseek(fd, nbloque*BLOCKSIZE, SEEK_SET);
    error = read(fd, buf, BLOCKSIZE);

    if(error != BLOCKSIZE){
        fprintf(stderr, "Error %d leyendo el bloque: %d", errno, nbloque);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

}
