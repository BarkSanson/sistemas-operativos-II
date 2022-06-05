#include "simulacion.h"

#define DEBUG13 1

struct INFORMACION {
    int pid;
    unsigned int nEscrituras;
    struct REGISTRO PrimeraEscritura;
    struct REGISTRO UltimaEscritura;
    struct REGISTRO MenorPosicion;
    struct REGISTRO MayorPosicion;
};