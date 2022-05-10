#include "directorios.h"

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

    // printf("RESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS\n");
    // bloqueReservado = reservar_bloque();
    // printf("Se ha reservado el bloque físico %d\n",bloqueReservado);

    // if(bread(posSB, SB) == ERROR_EXIT) {
    //     return 1;
    // }

    // printf("SB.cantBloquesLibres = %d\n",SB->cantBloquesLibres);
    // printf("Liberamos el bloque\n");
    // liberar_bloque(bloqueReservado);

    // if(bread(posSB, SB) == ERROR_EXIT) {
    //     return 1;
    // }

    // printf("Después de la liberación SB.cantBloquesLibres = %d",SB->cantBloquesLibres);

    // printf("MAPA DE BITS CON BLOQUES DE METADATOS OCUPADOS\n");

    
    // /*
    // printf("leer_bit(0) --> posbyte:0,posbit:0,nbloqueMB:0,nbloqueabs:1\n");
    // printf("leer_bit(0) = %d \n",leer_bit(0));
    // printf("leer_bit(1) --> posbyte:0,posbit:1,nbloqueMB:0,nbloqueabs:1\n");
    // printf("leer_bit(1) = %d \n",leer_bit(1));
    // printf("leer_bit(13) --> posbyte:1,posbit:5,nbloqueMB:0,nbloqueabs:1\n");
    // printf("leer_bit(13) = %d \n",leer_bit(13));
    // printf("leer_bit(14) --> posbyte:392,posbit:3,nbloqueMB:0,nbloqueabs:1\n");
    // printf("leer_bit(14) = %d \n",leer_bit(14));
    // printf("leer_bit(3138) --> posbyte:1,posbit:6,nbloqueMB:0,nbloqueabs:1\n");
    // printf("leer_bit(3138) = %d \n",leer_bit(3138));
    // printf("leer_bit(3139) --> posbyte:392,posbit:2,nbloqueMB:0,nbloqueabs:1\n");
    // printf("leer_bit(3139) = %d \n",leer_bit(3139));
    // printf("leer_bit(99999) --> posbyte:12499,posbit:7,nbloqueMB:12,nbloqueabs:13\n");
    // printf("leer_bit(99999) = %d \n",leer_bit(99999));
    // */
    
   
    // printf("leer_bit(0) --> posbyte:0,posbit:0,nbloqueMB:0,nbloqueabs:1\n");
    // printf("leer_bit(0) = %d \n",leer_bit(0));
    // printf("leer_bit(1) --> posbyte:0,posbit:1,nbloqueMB:0,nbloqueabs:1\n");
    // printf("leer_bit(1) = %d \n",leer_bit(1));
    // printf("leer_bit(123) --> posbyte:1,posbit:5,nbloqueMB:0,nbloqueabs:1\n");
    // printf("leer_bit(123) = %d \n",leer_bit(123));
    // printf("leer_bit(124) --> posbyte:392,posbit:3,nbloqueMB:0,nbloqueabs:1\n");
    // printf("leer_bit(124) = %d \n",leer_bit(124));
    // printf("leer_bit(31373) --> posbyte:1,posbit:6,nbloqueMB:0,nbloqueabs:1\n");
    // printf("leer_bit(31373) = %d \n",leer_bit(31373));
    // printf("leer_bit(31374) --> posbyte:392,posbit:2,nbloqueMB:0,nbloqueabs:1\n");
    // printf("leer_bit(31374) = %d \n",leer_bit(31374));
    // printf("leer_bit(31374) --> posbyte:392,posbit:2,nbloqueMB:0,nbloqueabs:1\n");
    // printf("leer_bit(31375) = %d \n",leer_bit(31375));
    // printf("leer_bit(31374) --> posbyte:392,posbit:2,nbloqueMB:0,nbloqueabs:1\n");
    // printf("leer_bit(999999) = %d \n",leer_bit(999999));
    // printf("leer_bit(999999) --> posbyte:392,posbit:2,nbloqueMB:0,nbloqueabs:1\n");


    // printf("%sLEYENDO INODO RAÍZ:%s\n", YELLOW, RESET_COLOR);
    // leer_inodo(0, &nodo);
    // mostrar_inodo(&nodo);

        
    // int ninodo = reservar_inodo('d',6);

    // traducir_bloque_inodo(ninodo,8,1);
    // traducir_bloque_inodo(ninodo,204,1);
    // traducir_bloque_inodo(ninodo,30004,1);
    // traducir_bloque_inodo(ninodo,400004,1);
    // traducir_bloque_inodo(ninodo,468750,1);

    // leer_inodo(1, &nodo);
    // mostrar_inodo(&nodo);
    // //hasta aqui funciona nivel 4 incluido
    
    // if(bread(posSB, SB) == ERROR_EXIT) {
    //     return 1;
    // }

    // printf("posPrimerInodoLibre = %d\n", SB->posPrimerInodoLibre);

    
    free(SB);
    bumount();

    return 0;
}