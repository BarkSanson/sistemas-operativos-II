#include "ficheros.h"

#define TAMNOMBRE 60

struct entrada{
    char nombre[TAMNOMBRE];
    unsigned int ninodo;
};

unsigned char extraer_camino(const char *camino,char *inicial,char *final,char *tipo);