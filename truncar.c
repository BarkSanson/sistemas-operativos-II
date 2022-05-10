/**
 * 
 * Autores: Arnau Vidal Moreno y Martín Ignacio Rizzo
 */ 
#include "ficheros.h"

void mostrar_stat(struct STAT* stat) {
    printf("stat.tamEnBytesLog = %d\n", stat->tamEnBytesLog);
    printf("stat.numBloquesOcupados = %d\n", stat->numBloquesOcupados);
}

int main(int argc, char** argv){
    char *path;
    int nbytes;
    int num_inodo;
    int bloquesInodo;
    int bloquesLiberados = 0;
    int bloquesLibres;
    struct inodo inodo;
    struct superbloque SB;
    struct STAT stat;

    if(argc != 4){
        fprintf(stderr, "Error al introducir el comando, la sintaxis debe ser [nombre_dispositivo] [ninodo] [nbytes]");
    }

    path = argv[1];
    num_inodo = atoi(argv[2]);
    nbytes = atoi(argv[3]);

    printf("%sEJECUTANDO TEST TRUNCAR.C%s\n", BOLD_GREEN, RESET_COLOR);

    bmount(path);
    leer_inodo(num_inodo,&inodo);
    bloquesInodo = inodo.numBloquesOcupados;
    bread(posSB,&SB);
    bloquesLibres = SB.cantBloquesLibres;

    printf("CANTIDAD BLOQUES LIBRES INICIAL SEGUN EL SUPERBLOQUE: %d",bloquesLibres);
    printf("BLOQUES OCUPADOS POR EL INODO INICIALMNETE: %d",bloquesInodo);


    if(nbytes == 0){
        liberar_inodo(num_inodo);
        #if DEBUG
            printf("LIBERANDO EL INODO %d",num_inodo);
        #endif
    } else {
        #if DEBUG
            printf("LIBERANDO LOS BLOQUES DEL INODO %d",num_inodo);
        #endif
        bloquesLiberados = mi_truncar_f(num_inodo,nbytes);
    }

    if(escribir_inodo(num_inodo, &inodo) == ERROR_EXIT) {
        fprintf(stderr, "[Error en mi_truncar.c()]: no se ha podido escribir el inodo %d\n", num_inodo);
        #if DEBUG
            fprintf(stderr, "%s<ERROR EN LA LÍNEA %d DE FICHEROS.C>%s", RED, __LINE__, RESET_COLOR);
        #endif
    }
    if(bwrite(posSB,&SB) == ERROR_EXIT) {
        fprintf(stderr, "[Error en mi_truncar.c()]: no se ha podido escribir el superbloque");
        #if DEBUG
            fprintf(stderr, "%s<ERROR EN LA LÍNEA %d DE FICHEROS.C>%s", RED, __LINE__, RESET_COLOR);
        #endif
    }

    mi_stat_f(num_inodo,&stat);
    mostrar_stat(&stat);

    bumount(path);

}