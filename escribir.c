#include "ficheros.h"

#define OFFSET1 9000
#define OFFSET2 209000
#define OFFSET3 30725000
#define OFFSET4 409605000
#define OFFSET5 480000000

void mostrar_stat(struct STAT* stat) {
    printf("stat.tamEnBytesLog = %d\n", stat->tamEnBytesLog);
    printf("stat.numBloquesOcupados = %d\n", stat->numBloquesOcupados);
}

void escribir_y_mostrar(
    unsigned int ninodo, 
    const void* buffer, 
    unsigned int offset, 
    unsigned int bytes) 
    {
    struct STAT stat;

    if(mi_write_f(ninodo, buffer, offset, sizeof(buffer)) == ERROR_EXIT) {
        fprintf(stderr, "[Error]: Error escribiendo en el inodo %d\n", ninodo);
    }

    mi_stat_f(ninodo, &stat);
    printf("Mostrando inodo %d con offset %d\n", ninodo, offset);
    mostrar_stat(&stat);
}

int main(int argc, char** argv) {
    int ninodo;
    
    if(argc < 4 || argc > 4) {
        printf("%s[Error en la sintaxis]:%s la sintaxis del comando ha de ser:\n\tescribir <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_inodos>\n", 
        RED, 
        RESET_COLOR);
        
        return 0;
    }

    bmount(argv[1]);

    if(atoi(argv[3]) == 0) { // Si es 0, reservamos sólo un inodo
        ninodo = reservar_inodo('f', 6);

        escribir_y_mostrar(ninodo, argv[2], OFFSET1, sizeof(argv[2]));
        escribir_y_mostrar(ninodo, argv[2], OFFSET2, sizeof(argv[2]));
        escribir_y_mostrar(ninodo, argv[2], OFFSET3, sizeof(argv[2]));
        escribir_y_mostrar(ninodo, argv[2], OFFSET4, sizeof(argv[2]));
        escribir_y_mostrar(ninodo, argv[2], OFFSET5, sizeof(argv[2]));
    }

    bumount();
    return 0;
}
