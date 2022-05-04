/**
 * 
 * Autores: Arnau Vidal Moreno y Martín Ignacio Rizzo
 */ 
#include "ficheros.h"
#include "ficheros_basico.h"
#include "bloques.h"

int main(int argc, char** argv){
    char *path;
    int nbytes;
    int num_inodo;
    int bloquesInodo;
    int bloquesLiberados = 0;
    int bloquesLibres;
    struct inodo inodo;
    struct superbloque SB;
    struct STAT stat;

    if(argc != 4){
        printf(stderr, "Error al introducir el comando, la sintaxis debe ser [nombre_dispositivo] [ninodo] [nbytes]");
    }
    path = argv[1];
    num_inodo = atoi(argv[2]);
    nbytes = atoi(argv[3]);

    printf("%sEJECUTANDO TEST TRUNCAR.C%s\n", BOLD_GREEN, RESET_COLOR);

    bmount(path);
    leer_inodo(num_inodo,&inodo);
    bloquesInodo = inodo.numBloquesOcupados;
    bread(posSB,&SB);
    bloquesLibres = SB.cantBloquesLibres;

    printf("CANTIDAD BLOQUES LIBRES INICIAL SEGUN EL SUPERBLOQUE: %d",bloquesLibres);
    printf("BLOQUES OCUPADOS POR EL INODO INICIALMNETE: %d",bloquesInodo);

    if(nbytes == 0){
        liberar_inodo(num_inodo);
        #if DEBUG
            printf("LIBERANDO EL INODO %d",num_inodo);
        #endif
    } else {
        #if DEBUG
            printf("LIBERANDO LOS BLOQUES DEL INODO %d",num_inodo);
        #endif
        bloquesLiberados = mi_truncar_f(num_inodo,nbytes);
    }
    if(bloquesInodo - bloquesLiberados != inodo.numBloquesOcupados){
        printf(stderr,"ERROR EN TRUNCAR.C, EL NÚMERO DE BLOQUES DEL INODO NO SE CORRESPONDE CON LOS BLOQUES LIBERADOS");
        #if DEBUG
            printf("BLOQUES LIBERADOS: %d, BLOQUES INICIALES: %d, BLOQUES FINALES: %d",bloquesLiberados,bloquesInodo,inodo.numBloquesOcupados);
        #endif
    }

    if(inodo.tamEnBytesLog != nbytes){
        printf(stderr,"ERROR EN TRUNCAR.C, TAMAÑO INODO NO COINCIDE CON NÚMERO DE BYTES A TRUNCAR");
        #if DEBUG
            printf("TAMAÑO INODO: %d, NUMERO DE BYTES: %d",inodo.tamEnBytesLog,nbytes);
        #endif
    }

    if(SB.cantBloquesLibres != bloquesLibres - bloquesLiberados){
        printf(stderr,"ERROR EN TRUNCAR.C, TAMAÑO SUPERBLOQUE NO COINCIDE CON EL NÚMERO DE BLOQUES LIBERADOS");
        #if DEBUG
            printf("TAMAÑO SUPERBLOQUE INICIAL: %d, BLOQUES LIBERADOS: %d",bloquesLibres,bloquesLiberados);
        #endif
    }

    if(escribir_inodo(num_inodo, &inodo) == ERROR_EXIT) {
        fprintf(stderr, "[Error en mi_truncar.c()]: no se ha podido escribir el inodo %d\n", num_inodo);
        #if DEBUG
            fprintf(stderr, "%s<ERROR EN LA LÍNEA %d DE FICHEROS.C>%s", RED, __LINE__, RESET_COLOR);
        #endif
    }
    if(bwrite(posSB,&SB) == ERROR_EXIT) {
        fprintf(stderr, "[Error en mi_truncar.c()]: no se ha podido escribir el superbloque");
        #if DEBUG
            fprintf(stderr, "%s<ERROR EN LA LÍNEA %d DE FICHEROS.C>%s", RED, __LINE__, RESET_COLOR);
        #endif
    }

    mi_stat_f(num_inodo,&stat);
    printf("NÚMERO DE BYTES DEL STAT %d",stat.tamEnBytesLog);
    printf("NÚMERO DE BLOQUES OCUPADOS DEL STAT %d",stat.numBloquesOcupados);

    bumount(path);

}