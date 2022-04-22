#include "ficheros_basico.h"

#define YELLOW  "\033[0;33m" 
#define BOLD_GREEN "\033[1;32m"
#define RESET_COLOR "\033[0m"

#define BLOQUE1 8
#define BLOQUE2 204
#define BLOQUE3 30004
#define BLOQUE4 400004
#define BLOQUE5 468750

void mostrar_inodo(struct inodo* inodo) {
    struct tm* ts;
    char atime[80];
    char mtime[80];
    char ctime[80];

    printf("Tipo: %c\n", inodo->tipo);
    printf("Permisos: %d\n", inodo->permisos);

    ts = localtime(&inodo->atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo->mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo->ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
    printf("atime: %s\n", atime);
    printf("mtime: %s\n", mtime);
    printf("ctime: %s\n", ctime);

    printf("nlinks: %d\n", inodo->nlinks);
    printf("tamEnBytesLog: %d\n", inodo->tamEnBytesLog);
    printf("numBloquesOcupados: %d\n", inodo->numBloquesOcupados);
}

int main(int argc, char **argv) {
    struct superbloque* SB = malloc(sizeof(struct superbloque));
    // struct inodo nodo;
    int posInodoReservado;

    if(argc != 2) {
        fprintf(stderr, "Número de argumentos inválido: son necesarios 1 argumento con la forma \n\tleer_sf <fichero>");
        return 1;
    }

    printf("%sEJECUTANDO TEST LEER_SF.C%s\n", BOLD_GREEN, RESET_COLOR);

    bmount(argv[1]);

    if(bread(posSB, SB) == ERROR_EXIT) {
        return 1;
    }

    printf("%sDATOS DEL SUPERBLOQUE%s:\n", YELLOW, RESET_COLOR);
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

    // === PRUEBAS DEL MB, DIRECTORIO RAÍZ Y RESERVA/LIBERACIÓN DE BLOQUE ===

    // (comentadas ya que a partir del nivel 4 ya no son necesarias)

    // printf("%sRESERVANDO INODO RAÍZ\n%s", YELLOW, RESET_COLOR);
    // reservar_inodo('d', 7);

    // if(bread(posSB, SB) == ERROR_EXIT) {
    //     bumount();
    //     return 1;
    // }

    // printf("cantInodosLibres después de reservar inodo raíz = %d\n", SB->cantInodosLibres);

    // printf("%sTAMANYOS DE CADA STRUCT:%s\n", YELLOW, RESET_COLOR);
    // printf("sizeof struct superbloque %ld\n", sizeof(struct superbloque));
    // printf("sizeof struct inodo %ld\n", sizeof(struct inodo));

    // // Mostramos el MB
    // printf("%sFRONTERAS DEL MB:%s\n", YELLOW, RESET_COLOR);
    // printf("leer_bit(%d) = %d\n", 0, leer_bit(0));
    // printf("leer_bit(%d) = %d\n", SB->posPrimerBloqueMB, leer_bit(SB->posPrimerBloqueMB));
    // printf("leer_bit(%d) = %d\n", SB->posUltimoBloqueMB, leer_bit(SB->posUltimoBloqueMB));
    // printf("leer_bit(%d) = %d\n", SB->posPrimerBloqueAI, leer_bit(SB->posPrimerBloqueAI));
    // printf("leer_bit(%d) = %d\n", SB->posUltimoBloqueAI, leer_bit(SB->posUltimoBloqueAI));
    // printf("leer_bit(%d) = %d\n", SB->posPrimerBloqueDatos, leer_bit(SB->posPrimerBloqueDatos));
    // printf("leer_bit(%d) = %d\n", SB->posUltimoBloqueDatos, leer_bit(SB->posUltimoBloqueDatos));

    // printf("%sLEYENDO INODO RAÍZ:%s\n", YELLOW, RESET_COLOR);
    // leer_inodo(0, &nodo);
    // mostrar_inodo(&nodo);

    // printf("%sRESERVAMOS Y LIBERAMOS UN BLOQUE:%s\n", YELLOW, RESET_COLOR);
    // if(bread(posSB, SB) == ERROR_EXIT) {
    //     bumount();
    //     return 1;
    // }
    // printf("cantBloquesLibres antes de reservar = %d\n", SB->cantBloquesLibres);
    // reservar_bloque();
    // if(bread(posSB, SB) == ERROR_EXIT) {
    //     bumount();
    //     return 1;
    // }
    // printf("cantBloquesLibres despues de reservar= %d\n", SB->cantBloquesLibres);

    // liberar_bloque(7);

    // if(bread(posSB, SB) == ERROR_EXIT) {
    //     bumount();
    //     return 1;
    // }
    // printf("cantBloquesLibres despues de liberar = %d\n", SB->cantBloquesLibres);

    // === PRUEBAS TRADUCIR_BLOQUE_INODO === 
    posInodoReservado = reservar_inodo('f', 6);
    traducir_bloque_inodo(posInodoReservado, BLOQUE1, 1);

    
    free(SB);
    bumount();

    return 0;
}