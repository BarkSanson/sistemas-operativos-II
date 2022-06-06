/**
 * Autores: Arnau Vidal Moreno y Martín Ignacio Rizzo
 */ 
#include "ficheros.h"

#define TAM_BUFFER 1500

int main(int argc, char** argv) {
    struct STAT stat;
    int totalLeidos;
    int leidosActual;
    int offset;
    unsigned int ninodo;
    char buffer[TAM_BUFFER];

    if(argc != 3) {
        printf("%sleer.c:%s la sintaxis del comando ha de ser:\n\ttest <nombre_dispositivo> <ninodo>\n", 
        RED, 
        RESET_COLOR);
        
        return 1;
    }

    fprintf(stderr, "%sEJECUTANDO TEST LEER.C%s\n", BOLD_GREEN, RESET_COLOR);

    offset = 0;
    totalLeidos = 0;
    ninodo = atoi(argv[2]);

    bmount(argv[1]);

    mi_stat_f(ninodo, &stat);
    memset(buffer, 0, TAM_BUFFER);

    while((leidosActual = mi_read_f(ninodo, buffer, offset, TAM_BUFFER)) > 0 && (totalLeidos < stat.tamEnBytesLog)) {
        write(1, buffer, leidosActual);
        totalLeidos += leidosActual;
        offset += TAM_BUFFER;
        memset(buffer, 0, TAM_BUFFER);
    }

    #if DEBUG5
        fprintf(stderr, "\nTotal de bytes leidos: %d\n", totalLeidos);
        fprintf(stderr, "tamEnBytesLog del inodo leido: %d\n", stat.tamEnBytesLog);
    #endif

    bumount();
    return 0;
}