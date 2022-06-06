/**
 * Autores: Arnau Vidal Moreno y Martín Ignacio Rizzo
 */ 
#include "bloques.h"
#include "semaforo_mutex_posix.h"

static int fd;
static sem_t* mutex;
static unsigned int inside_sc = 0;
static int tamSFM; // Tamanyo de memoria compartida
static void* ptrSFM; // Puntero a memoria compartida

/**
 * Mapea un fichero en memoria
 * 
 * @param   fd  Descriptor del fichero a mapear
 * 
 * @returns Un puntero hacia la memoria compartid
 */ 
void* do_mmap(int fd) {
    struct stat st;
    void* ptr;

    fstat(fd, &st);
    tamSFM = st.st_size;

    if((ptr = mmap(NULL, tamSFM, PROT_WRITE, MAP_SHARED, fd, 0)) == (void*) - 1)
        fprintf(stderr, "[Error en do_mmap()]: error %d: %s\n", errno, strerror(errno));

    return ptr;
}

/**
 * Inicializa el flujo de datos, nuestro disco
 * 
 * @param   camino  ruta donde establecer el fd
 * @returns     SUCCESS_EXIT si ha funcionado bien
 *              ERROR_EXIT si ha habido un error
*/
int bmount(const char *camino) {
    // Inicializamos el semáforo
    if(!mutex) {
        mutex = initSem();
        if(mutex == SEM_FAILED) {
            return ERROR_EXIT;
        }
    }

    if(fd > 0) {
        close(fd);
    }

    // Inicializamos el dispositivo virtual
    umask(000);
    fd = open(camino, O_RDWR | O_CREAT, 0666);
    if(fd == -1) {
        fprintf(stderr, "Error %d montando el fichero: %s\n", errno, strerror(errno));
        return ERROR_EXIT;
    }

    ptrSFM = do_mmap(fd);

    return SUCCESS_EXIT;
}

/**
 * Cierra el descriptor de fichero, y por tanto nuestro
 * sistema de ficheros
 * 
 * @returns     SUCCESS_EXIT si ha funcionado bien,
 *              ERROR_EXIT si ha habido un error     
*/
int bumount() {
    // Eliminamos el semáforo
    deleteSem();

    // Volcamos a disco
    msync(ptrSFM, tamSFM, MS_ASYNC);

    // Liberamos la memoria ocupada por el fichero mapeado
    munmap(ptrSFM, tamSFM);

    fd = close(fd);

    // Cerramos el dispositivo virtual
    if(close(fd)== -1){
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
 * @returns     SUCCESS_EXIT si ha funcionado bien,
 *              ERROR_EXIT si ha habido un error 
*/
int bwrite(unsigned int nbloque, const void *buf) {
    int tam;

    if(nbloque * BLOCKSIZE <= tamSFM) {
        tam = BLOCKSIZE;
    } else {
        tam = tamSFM - nbloque * BLOCKSIZE;
    }

    if(tam > 0) {
        memcpy(ptrSFM + nbloque*BLOCKSIZE, buf, tam);
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
    int tam;

    if(nbloque * BLOCKSIZE <= tamSFM) {
        tam = BLOCKSIZE;
    } else {
        tam = tamSFM - nbloque * BLOCKSIZE;
    }

    if(tam > 0) {
        memcpy(buf, ptrSFM + nbloque*BLOCKSIZE, tam);
    }

    return SUCCESS_EXIT;
}

/**
 * Da la senyal wait al semáforo
 */
void mi_waitSem() {
    if(!inside_sc) {
        waitSem(mutex);
    }
    inside_sc++;
}

/**
 * Da la senyal signal al semáforo
 */
void mi_signalSem() {
    inside_sc--;
    if(!inside_sc) {
        signalSem(mutex);
    }
}
