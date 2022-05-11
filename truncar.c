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
    int bloquesLibres;
    struct inodo inodo;
    struct superbloque SB;
    struct STAT stat;

    if(argc != 4){
        fprintf(stderr, "Error al introducir el comando, la sintaxis debe ser [nombre_dispositivo] [ninodo] [nbytes]");
        return 1;
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
        #if DEBUG6
            fprintf(stderr, "truncar.c -> Liberando el inodo %d",
            num_inodo);
        #endif
        if(liberar_inodo(num_inodo) == ERROR_EXIT) {
            fprintf(stderr, "truncar.c: Error al liberar el inodo %d",
            num_inodo);
            return 1;
        }
    } else {
        #if DEBUG6
            fprintf(stderr, 
            "truncar.c -> Liberando los bloques del inodo %d",
            num_inodo);
        #endif
        mi_truncar_f(num_inodo,nbytes);
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

    return 0;
}