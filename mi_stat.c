#include "directorios.h"

int main(int argc, char** argv) {
    struct STAT stat;
    int p_inodo;
    char* camino;
    char* disco;

    if(argc != 3) {
        fprintf(stderr, "mi_stat: Sintaxis incorrecta, la sintaxis debe seguir la siguiente forma:\n\t ./mi_stat <disco> <ruta>\n");
        return 1;
    }

    disco = argv[1];
    camino = argv[2];

    bmount(disco);

    p_inodo = mi_stat(camino, &stat);
    if(p_inodo == ERROR_EXIT) {
        fprintf(stderr, "mi_stat: Sintaxis incorrecta, la sintaxis debe seguir la siguiente forma:\n\t ./mi_stat <disco> <ruta>\n");
        return 1;
    }

    printf("Número de inodo: %d\n", p_inodo);
    printf("Tipo: %c\n", stat.tipo);
    printf("Permisos: %d\n", stat.permisos);

    struct tm* ts;
    char atime[80];
    char mtime[80];
    char ctime[80];

    ts = localtime(&stat.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S\n", ts);
    ts = localtime(&stat.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S\n", ts);
    ts = localtime(&stat.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S\n", ts);
    printf("atime: %s\n", atime);
    printf("mtime: %s\n", mtime);
    printf("ctime: %s\n", ctime);

    printf("nlinks: %d\n", stat.nlinks);
    printf("tamEnBytesLog: %d\n", stat.tamEnBytesLog);
    printf("numBloquesOcupados: %d\n", stat.numBloquesOcupados);

    bumount();

    return 0;
}