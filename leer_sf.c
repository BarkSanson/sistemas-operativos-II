#include "ficheros_basico.h"

int main(int argc, char **argv) {
    struct superbloque* SB = malloc(sizeof(struct superbloque));
    int nbloques;

    bmount(argv[1]);

    nbloques = atoi(argv[2]);

    initSB(nbloques, nbloques/4);
    initMB();
    initAI();

    if(bread(posSB, SB) == ERROR_EXIT) {
        bumount();
        exit(1);
    }

    printf("Ejecutando test leer_sf.c\n");
    printf("Datos del superbloque:\n");
    printf("posPrimerBLoqueMB = %d\n", SB->posPrimerBloqueMB);
    printf("posUltimoBLoqueMB = %d\n", SB->posUltimoBloqueMB);
    printf("posPrimerBloqueAI = %d\n", SB->posPrimerBloqueAI);
    printf("posUltimoBloqueAI = %d\n", SB->posUltimoBloqueAI);
    printf("posPrimerBloqueDatos = %d\n", SB->posPrimerBloqueDatos);
    printf("posUltimoBloqueDatos = %d\n", SB->posUltimoBloqueDatos);
    printf("posInodoRaiz = %d\n", SB->posInodoRaiz); 
    printf("posPrimerInodoLibre = %d\n", SB->posPrimerInodoLibre);
    printf("cantBloquesLibres = %d\n", SB->cantBloquesLibres);
    printf("cantInodosLibres = %d\n", SB->cantInodosLibres);
    printf("totBloques = %d\n", SB->totBloques); 
    printf("totInodos = %d\n", SB->totInodos);

    printf("sizeof struct superbloque %ld\n", sizeof(struct superbloque));
    printf("sizeof struct inodo %ld\n", sizeof(struct inodo));

    // Mostramos el MB
    for(int i = 0; i <= SB->posUltimoBloqueDatos; i++) {
        printf("leer_bit(%d) = %d\n", i, leer_bit(i));
    }

    free(SB);
    bumount();

    return 0;

}