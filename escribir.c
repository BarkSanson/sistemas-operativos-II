#include "ficheros.h"

#define OFFSET1 9000
#define OFFSET2 209000
#define OFFSET3 30725000
#define OFFSET4 409605000
#define OFFSET5 480000000

int main(int argc, char** argv) {
    int posInodo;
    
    if(argc < 4 || argc > 4) {
        printf("%s[Error en la sintaxis]:%s la sintaxis del comando ha de ser:\n\tescribir <nombre_dispositivo> <\"$(cat fichero)\"> <inodos>\n", 
        RED, 
        RESET_COLOR);
        
        return 0;
    }

    bmount(argv[1]);

    bumount();
    return 0;
}