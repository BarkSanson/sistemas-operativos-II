/**
 * 
 * Autores: Arnau Vidal Moreno y Martín Ignacio Rizzo
 */ 
#include "ficheros.h"

#define TAM_BUFFER 100 * BLOCKSIZE

int main(int argc, char** argv) {
    struct STAT stat;
    int totalLeidos;
    int leidosActual;
    int offset;
    unsigned int ninodo;
    char buffer[TAM_BUFFER];

    if(argc != 3) {
        printf("%s[Error en la sintaxis]:%s la sintaxis del comando ha de ser:\n\ttest <nombre_dispositivo> <ninodo>\n", 
        RED, 
        RESET_COLOR);
        
        return 1;
    }

    printf("%sEJECUTANDO TEST LEER.C%s\n", BOLD_GREEN, RESET_COLOR);

    offset = 0;
    ninodo = atoi(argv[2]);

    bmount(argv[1]);

    mi_stat_f(ninodo, &stat);
    memset(buffer, 0, TAM_BUFFER);

    fprintf(stderr, "ninodo = %d\n", ninodo);
    fprintf(stderr, "tamEnBytesLog = %d\n", stat.tamEnBytesLog);

    while((leidosActual = mi_read_f(ninodo, buffer, offset, TAM_BUFFER)) > 0 && (totalLeidos < stat.tamEnBytesLog)) {
        // fprintf(stderr, "Leyendo inodo %d con el offset %d\n", ninodo, offset);
        fwrite(buffer, sizeof(char), leidosActual, stdout);
        totalLeidos += leidosActual;
        offset += TAM_BUFFER;
        memset(buffer, 0, TAM_BUFFER);
    }

    printf("\nTotal de bytes leidos: %d\n", totalLeidos);
    printf("tamEnBytesLog del inodo leido: %d\n", stat.tamEnBytesLog);

    bumount();
    return 0;
}