#include "ficheros.h"

#define TAM_BUFFER 100 * BLOCKSIZE

int main(int argc, char** argv) {
    struct STAT stat;
    struct inodo inodo;
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

    // mi_stat_f(ninodo, &stat);
    leer_inodo(ninodo, &inodo);
    memset(buffer, 0, TAM_BUFFER);

    fprintf(stderr, "ninodo = %d\n", ninodo);
    fprintf(stderr, "tamEnBytesLog = %d\n", inodo.tamEnBytesLog);

    while((leidosActual = mi_read_f(ninodo, buffer, offset, TAM_BUFFER)) > 0 && (totalLeidos < inodo.tamEnBytesLog)) {
        // fprintf(stderr, "Leyendo inodo %d con el offset %d\n", ninodo, offset);
        fwrite(buffer, sizeof(char), leidosActual, stdout);
        totalLeidos += leidosActual;
        offset += TAM_BUFFER;
        memset(buffer, 0, TAM_BUFFER);
    }

    printf("Total de bytes leidos: %d\n", totalLeidos);
    printf("tamEnBytesLog del inodo leido: %d\n", inodo.tamEnBytesLog);

    bumount();
    return 0;
}