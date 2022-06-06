/**
 * Autores: Arnau Vidal Moreno y Martín Ignacio Rizzo
 */ 
#include "simulacion.h"

static int acabados = 0;

void reaper() {
    pid_t ended;
    signal(SIGCHLD, reaper);
    while((ended = waitpid(-1, NULL, WNOHANG)) > 0) {
        acabados++;
    }
}

int main(int argc, char** argv) {
    struct tm* tm;
    time_t t;
    int pid;
    char fecha[80];
    char cadena_simul_fecha[25];
    char* disco;

    if(argc != 2) {
        fprintf(stderr, "simulacion.c: sintaxis incorrecta, la sintaxis debe"
        " seguir la siguiente forma:\n\t./simulacion <disco>\n");
        return 1;
    }

    // Asignamos la función reaper a la
    // senyal SIGCHLD
    signal(SIGCHLD, reaper);

    disco = argv[1];

    bmount(disco);

    t = time(NULL);
    tm = localtime(&t);
    strftime(fecha, sizeof(fecha), "%Y%m%d%H%M%S", tm);

    strcpy(cadena_simul_fecha, "/simul_");
    strcat(cadena_simul_fecha, fecha);
    strcat(cadena_simul_fecha, "/");

    // Crear directorio de la simulación
    if(mi_creat(cadena_simul_fecha, 6) == ERROR_EXIT) {
        return 1;
    }

    for(int i = 1; i <= NUMPROCESOS; i++) {
        pid = fork();
        
        // Es el hijo
        if(pid == 0) {
            char nombre_directorio[50];
            char nombre_fichero[100];
            bmount(disco);
        
            sprintf(nombre_directorio, "%sproceso_%d/", cadena_simul_fecha, getpid());
        
            if(mi_creat(nombre_directorio, 6) == ERROR_EXIT) {
                return 1;
            }

            sprintf(nombre_fichero, "%sprueba.dat", nombre_directorio);

            if(mi_creat(nombre_fichero, 6) == ERROR_EXIT) {
                fprintf(
                    stderr, 
                    "simulacion.c: no se ha podido crear el fichero de nombre %s", 
                    nombre_fichero);
                bumount();
                exit(1);
            }

            srand(time(NULL) + getpid());

            for(int j = 1; j <= NUMESCRITURAS; j++) {
                struct REGISTRO registro;

                registro.fecha = time(NULL);
                registro.nEscritura = j;
                registro.pid = getpid();
                registro.nRegistro = rand() % REGMAX;

                #if DEBUG12
                    fprintf(
                        stderr, 
                        "[simulacion.c -> Escribiendo registro con PID %d,"
                        "nEscritura %d, nRegistro %d, fecha %s\n",
                        registro.pid,
                        registro.nEscritura,
                        registro.nRegistro,
                        asctime(localtime(&registro.fecha))
                    );
                #endif

                if(mi_write(
                    nombre_fichero, 
                    &registro, 
                    registro.nRegistro * sizeof(struct REGISTRO), 
                    sizeof(struct REGISTRO)) == ERROR_EXIT) {

                    exit(1);
                }

                #if DEBUG12
                    fprintf(stderr, "[simulacion.c -> Escritura %d en %s\n", j, nombre_fichero);
                #endif

                usleep(50000);
            }
            #if DEBUG12
                fprintf(stderr, "[simulacion.c -> Proceso %d completadas %d escrituras\n", getpid(), NUMESCRITURAS);
            #endif

            bumount();
            exit(0);        

        }

        usleep(150000);
    }

    while(acabados < NUMPROCESOS) {
        pause();
    }

    bumount();
    return 0;
}