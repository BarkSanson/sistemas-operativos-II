#include "ficheros.h"

int main(int argc, char** argv) {
    if(argc != 3) {
        printf("%s[Error en la sintaxis]:%s la sintaxis del comando ha de ser:\n\ttest <nombre_dispositivo> <ninodo>\n", 
        RED, 
        RESET_COLOR);
        
        return 0;
    }
    return 0;
}