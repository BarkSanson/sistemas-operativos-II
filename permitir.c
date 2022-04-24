#include "ficheros_basico.h"
#include "ficheros.c"

int main(int argc, char** argv) {
    char *path;
    int ninodo;

    if(argc != 3) {
        fprintf(stderr, "Número de argumentos inválido, son necesarios 3 argumentos: [nombre_dispositivo] [número_de_inodo] [permisos_inodo]");
        return EXIT_FAILURE;
    }
     
    //montamos el dispositivo 
    path = argv[1];
    bmount(path);

    //transformamos a int el numero de inodo
    ninodo = atoi(argv[2]);

    //llamamos a la funcion mi_chmod_f dandole el numero de inodo
    //y sus correspondientes permisos
    mi_chmod_f(ninodo,argv[3]);

    //desmontamos el dispositivo
    bumount();
}