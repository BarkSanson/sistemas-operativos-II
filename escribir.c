#include "ficheros.h"

int main(int argc, char** argv) {
    
    if(argc < 3 || argc > 3) {
        printf("%s[Error en la sintaxis]:%s la sintaxis del comando ha de ser:\n\tescribir <nombre_dispositivo> <fichero> <inodos>", 
        RED, 
        RESET_COLOR);
        
        return 0;
    }

    bmount(argv[2]);
    

    bumount();
    return 0;
}