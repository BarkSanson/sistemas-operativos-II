#include "ficheros.h"

#define TAM_BUFFER 1500

int main(int argc, char** argv) {
    struct STAT stat;
    int leidos;
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

    leidos = mi_read_f(ninodo, buffer, offset, TAM_BUFFER);
    while(leidos > 0) {
        offset += TAM_BUFFER;
        memset(buffer, 0, leidos);
        leidos += mi_read_f(ninodo, buffer, offset, TAM_BUFFER);
        write(1, buffer, leidos);
    }

    mi_stat_f(ninodo, &stat);

    printf("Total de bytes leidos: %d\n", leidos);
    printf("tamEnBytesLog del inodo leido: %d\n", stat.tamEnBytesLog);


    bumount();
    return 0;
}