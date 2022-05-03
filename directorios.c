#include "directorios.h"

/**
 * @brief dado un path o camino este se divide en dos, la ruta de la carpeta
 * o fichero y el propio nombre del fichero
 * 
 * @param camino entrada que debemos cortar
 * @param inicial 
 * @param tipo 
 * @return f si es un fichero o d si es un directorio
 */
unsigned char extraer_camino(const char *camino,char *inicial,char *final,char *tipo){
    char *token;
    int i = 0;
    
    //encuentra el primer token
    token = strtok(camino,'/');

    while(token != NULL){
        
    }
}