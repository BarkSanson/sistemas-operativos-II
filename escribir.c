#include "ficheros.h"

#define NOFFSETS    5
#define OFFSET1     9000
#define OFFSET2     209000
#define OFFSET3     30725000
#define OFFSET4     409605000
#define OFFSET5     480000000

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

    if(mi_write_f(ninodo, buffer, offset, strlen(buffer)) == ERROR_EXIT) {
        fprintf(stderr, "[Error]: Error escribiendo en el inodo %d\n", ninodo);
    }

    mi_stat_f(ninodo, &stat);
    printf("Mostrando inodo %d con offset %d\n", ninodo, offset);
    mostrar_stat(&stat);
}

int main(int argc, char** argv) {
    int ninodo;
    char diferentesInodos;
    char* texto;
    char ninodos[NOFFSETS];
    
    if(argc != 4) {
        printf("%s[Error en la sintaxis]:%s la sintaxis del comando ha de ser:\n\tescribir <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_inodos>\n", 
        RED, 
        RESET_COLOR);
        
        return 0;
    }

    texto = malloc(strlen(argv[2]));
    strcpy(texto, argv[2]);
    diferentesInodos = atoi(argv[3]);
    
    printf("%sEJECUTANDO TEST ESCRIBIR.C%s\n", BOLD_GREEN, RESET_COLOR);
    printf("Tamanyo de la entrada: %ld\n", strlen(texto));

    bmount(argv[1]);

    if(diferentesInodos == 0) { // Si es 0, reservamos sólo un inodo
        ninodo = reservar_inodo('f', 6);

        escribir_y_mostrar(ninodo, texto, OFFSET1, strlen(texto));
        escribir_y_mostrar(ninodo, texto, OFFSET2, strlen(texto));
        escribir_y_mostrar(ninodo, texto, OFFSET3, strlen(texto));
        escribir_y_mostrar(ninodo, texto, OFFSET4, strlen(texto));
        escribir_y_mostrar(ninodo, texto, OFFSET5, strlen(texto));
    } else if(diferentesInodos == 1) {
        for(int i = 0; i < NOFFSETS; i++) {
            ninodos[i] = reservar_inodo('f', 7);
        }

        escribir_y_mostrar(ninodos[0], texto, OFFSET1, strlen(texto));
        escribir_y_mostrar(ninodos[1], texto, OFFSET2, strlen(texto));
        escribir_y_mostrar(ninodos[2], texto, OFFSET3, strlen(texto));
        escribir_y_mostrar(ninodos[3], texto, OFFSET4, strlen(texto));
        escribir_y_mostrar(ninodos[4], texto, OFFSET5, strlen(texto));
    }

    bumount();
    return 0;
}
