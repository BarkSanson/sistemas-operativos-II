/**
 * 
 * Autores: Arnau Vidal Moreno y Martín Ignacio Rizzo
 */ 
#include "ficheros.h"

void mostrar_stat(struct STAT* stat) {
    printf("stat.tipo: %c\n", stat->tipo);
    printf("stat.tamEnBytesLog = %d\n", stat->tamEnBytesLog);
    printf("stat.numBloquesOcupados = %d\n", stat->numBloquesOcupados);
}

int main(int argc, char** argv){
    char *path;
    int nbytes;
    int num_inodo;
    struct STAT stat;

    if(argc != 4){
        fprintf(stderr, "truncar.c: Error al introducir el comando, la sintaxis debe ser: \n\ttruncar [nombre_dispositivo] [ninodo] [nbytes]\n");
        return 1;
    }

    path = argv[1];
    num_inodo = atoi(argv[2]);
    nbytes = atoi(argv[3]);

    printf("%sEJECUTANDO TEST TRUNCAR.C%s\n", BOLD_GREEN, RESET_COLOR);

    bmount(path);

    mi_stat_f(num_inodo, &stat);

    if(nbytes == 0){
        #if DEBUG6
            fprintf(stderr, "truncar.c -> Liberando el inodo %d\n",
            num_inodo);
        #endif
        if(liberar_inodo(num_inodo) == ERROR_EXIT) {
            fprintf(stderr, "truncar.c: Error al liberar el inodo %d\n",
            num_inodo);
            return 1;
        }
    } else {
        #if DEBUG6
            fprintf(stderr, 
            "truncar.c -> Liberando los bloques del inodo %d\n",
            num_inodo);
        #endif
        mi_truncar_f(num_inodo,nbytes);
    }

    mi_stat_f(num_inodo,&stat);
    mostrar_stat(&stat);

    bumount(path);

    return 0;
}