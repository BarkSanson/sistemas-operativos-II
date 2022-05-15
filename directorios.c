#include "directorios.h"

/**
 * @brief dado un path o camino este se divide en dos, la ruta de la carpeta
 * o fichero y el propio nombre del fichero
 * 
 * @param camino    entrada que debemos cortar
 * @param inicial   Puntero al string que contendrá la parte inicial del camino
 * @param final     Puntero al string que contendrá el camino, menos inicial
 * @param tipo      Tipo de inodo (fichero o directorio)
 * @return f si es un fichero o d si es un directorio
 */
unsigned char extraer_camino(const char *camino,char *inicial,char *final,char *tipo){
    char* resto;
    
    if(camino[0] != '/') {
        fprintf(stderr, "[Error en extraer_camino()]: el camino no puede estar vacío y debe empezar por /");
        return ERROR_EXIT;
    }

    // Localizamos la primera / después
    // de la inicial
    resto = strchr(camino + 1, '/');

    if(resto) {
        // Si resto no es NULL,
        // significa que hay segundo /
        strncpy(inicial, camino + 1, strlen(camino) - strlen(resto) - 1);
        strcpy(final, resto);
        strcpy(tipo, "d");

        return 'd';
    } else {
        strcpy(inicial, camino + 1);
        strcpy(final, "");
        strcpy(tipo, "f");

        return 'f';
    }

    // Si llega aquí es porque no es ni fichero ni
    // directorio; ha habido un error en algún sitio
    return ERROR_EXIT; 
}