#include "directorios.h"
#include <sys/wait.h>
#include <signal.h>

#define DEBUG12 1

#define NUMPROCESOS 100
#define NUMESCRITURAS 50
#define REGMAX 500000

struct REGISTRO {
    time_t fecha;
    pid_t pid;
    int nEscritura;
    int nRegistro;
};

void reaper();