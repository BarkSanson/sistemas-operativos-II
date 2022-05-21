/**
 * Autores: Arnau Vidal Moreno y Martín Ignacio Rizzo
 */ 
#include "ficheros.h"

#define DEBUGESCRIBIR 0

void mostrar_stat(struct STAT* stat) {
    fprintf(stderr, "stat.tamEnBytesLog = %d\n", stat->tamEnBytesLog);
    fprintf(stderr, "stat.numBloquesOcupados = %d XD LOL\n", stat->numBloquesOcupados);
}

int main(int argc, char** argv) {
    struct STAT stat;
    int ninodo;
    char diferentesInodos;
    char* texto;
    int offsets[5] = {9000, 209000, 30725000, 409605000, 480000000};

    if(argc != 4) {
        printf("%s[Error en la sintaxis]:%s la sintaxis del comando ha de ser:\n\tescribir <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_inodos>\n", 
        RED, 
        RESET_COLOR);
        
        return 1;
    }

    texto = malloc(strlen(argv[2]));
    strcpy(texto, argv[2]);
    diferentesInodos = atoi(argv[3]);
    
    fprintf(stderr, "%sEJECUTANDO TEST ESCRIBIR.C%s\n", BOLD_GREEN, RESET_COLOR);
    fprintf(stderr, "Tamanyo de la entrada: %ld\n", strlen(texto));

    bmount(argv[1]);

    if(diferentesInodos == 0) { // Si es 0, reservamos sólo un inodo
        ninodo = reservar_inodo('f', 6);

        for(int i = 0; i < sizeof(offsets) / sizeof(int); i++) {
            int bytesEscritos = mi_write_f(ninodo, texto, offsets[i], strlen(texto));
            if(bytesEscritos == ERROR_EXIT) {
                fprintf(stderr, 
                "escribir.c: Error escribiendo el texto en el inodo %d con offset %d\n",
                ninodo,
                offsets[i]);
                return 1;
            }

            fprintf(stderr, "Bytes escritos: %d\n", bytesEscritos);
            fprintf(stderr, "Mostrando inodo %d con offset %d\n",
            ninodo,
            offsets[i]);
            mi_stat_f(ninodo, &stat);
            mostrar_stat(&stat);
        }

    } else if(diferentesInodos == 1) {
        for(int i = 0; i < sizeof(offsets) / sizeof(int); i++) {
            ninodo = reservar_inodo('f', 6);

            int bytesEscritos = mi_write_f(ninodo, texto, offsets[i], strlen(texto));
            if(bytesEscritos == ERROR_EXIT) {
                fprintf(stderr, 
                "escribir.c: Error escribiendo el texto en el inodo %d con offset %d\n",
                ninodo,
                offsets[i]);
                return 1;
            }

            #if DEBUGESCRIBIR
                char* buffer[strlen(texto)];
                memset(buffer, 0, strlen(texto));
                int bytesLeidos = mi_read_f(ninodo, buffer, offsets[i], strlen(texto));
                write(1, buffer, strlen(texto)); 
                printf("\n[DEBUG]: Bytes leidos en escribir.c: %d", bytesLeidos);
            #endif

            fprintf(stderr, "Bytes escritos: %d\n", bytesEscritos);
            fprintf(stderr, "Mostrando inodo %d con offset %d\n",
            ninodo,
            offsets[i]);
            mi_stat_f(ninodo, &stat);
            mostrar_stat(&stat);
        }
    }

    bumount();
    return 0;
}
