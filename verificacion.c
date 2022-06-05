#include "verificacion.h"

int main(int argc, char** argv) {
    struct STAT stat;
    struct entrada entradas[NUMPROCESOS * sizeof(struct entrada)];
    char* dispositivo;
    char* directorio_simulacion;
    unsigned int numEntradas;

    if(argc != 3) {
        fprintf(stderr, "verificacion.c: sintaxis incorrecta, la sintaxis debe"
        " seguir la siguiente forma:\n\t./verificacion <nombre_dispositivo> <directorio_simulacion>\n");
        return 1;
    }

    dispositivo = argv[1];
    directorio_simulacion = argv[2];

    if(*(directorio_simulacion + strlen(directorio_simulacion) - 1) != '/') {
        fprintf(stderr, "verificacion.c: no se ha introducido un directorio válido");
        return 1;
    }

    bmount(dispositivo);

    if(mi_stat(directorio_simulacion, &stat) == ERROR_EXIT) {
        fprintf(stderr, "verificacion.c: no se ha podido leer "
                "el stat del directorio de simulación");
        return 1;
    }

    numEntradas = stat.tamEnBytesLog/sizeof(struct entrada);

    if(numEntradas != NUMPROCESOS) {
        fprintf(stderr, "verificacion.c: el número de entradas no coincide"
                "con el número de procesos");
        return 1;
    }

    char fichero_informe[strlen(directorio_simulacion) + strlen("informe.txt")];
    sprintf(fichero_informe, "%sinforme.txt", directorio_simulacion);

    if(mi_creat(fichero_informe, 6) == ERROR_EXIT) {
        fprintf(stderr, "verificacion.c: no se ha podido crear el fichero informe.txt");
        return 1;
    
    }

    if(mi_read(directorio_simulacion, entradas, 0, sizeof(entradas)) == ERROR_EXIT) {
        fprintf(stderr, "verificacion.c: error leyendo las entradas del directorio");
        return 1;
    }

    int offset_informe = 0;
    for(int i = 0; i < numEntradas; i++) {
        struct INFORMACION info;
        pid_t pid;
        char fichero_prueba[
            strlen(directorio_simulacion) + 
            strlen(entradas[i].nombre) + 
            strlen("prueba.dat")];
        int cant_registros_buffer_escrituras = 256 * 24;
        int offset;
        char buffer[BLOCKSIZE];

        pid = atoi(strchr(entradas[i].nombre, '_') + 1);
        info.pid = pid;
        info.nEscrituras = 0;

        sprintf(
            fichero_prueba, 
            "%s%s/prueba.dat", 
            directorio_simulacion, 
            entradas[i].nombre);

        struct REGISTRO buffer_escrituras[cant_registros_buffer_escrituras];
        memset(buffer_escrituras, 0, sizeof(buffer_escrituras));

        // Recorremos las escrituras de prueba.dat
        offset = 0;
        while(mi_read(fichero_prueba, buffer_escrituras, offset, sizeof(buffer_escrituras)) > 0) {
            for(int i = 0; i < cant_registros_buffer_escrituras; i++) {
                // Si la estructura no es válida, seguimos leyendo
                if(info. pid!= buffer_escrituras[i].pid) {
                    if(info.nEscrituras == 0) {
                        info.MenorPosicion = buffer_escrituras[i];
                        info.MayorPosicion = buffer_escrituras[i];
                        info.PrimeraEscritura = buffer_escrituras[i];
                        info.UltimaEscritura = buffer_escrituras[i];
                        info.nEscrituras++;
                    } else {
                        // Para actualizar PrimeraEscrutura o UltimaEscritura,
                        // comprobamos que el nEscritura del registro leido sea menor/mayor que el guardado
                        if(buffer_escrituras[i].nEscritura < info.PrimeraEscritura.nEscritura) {
                            info.PrimeraEscritura = buffer_escrituras[i];
                        }
                        if(buffer_escrituras[i].nEscritura > info.UltimaEscritura.nEscritura) {
                            info.UltimaEscritura = buffer_escrituras[i];
                        }

                        // Lo mismo para actualizar MenorPosicion o MayorPosición,
                        // pero con el nRegistro
                        if(buffer_escrituras[i].nRegistro < info.MenorPosicion.nRegistro) {
                            info.MenorPosicion = buffer_escrituras[i];
                        }
                        if(buffer_escrituras[i].nRegistro > info.MayorPosicion.nRegistro) {
                            info.MayorPosicion = buffer_escrituras[i];
                        }
                        info.nEscrituras++;
                    }
                }

                memset(buffer_escrituras, 0, sizeof(buffer_escrituras));
                offset += sizeof(buffer_escrituras);
                // #if DEBUG13
                //     fprintf(stderr, "[verificacion.c -> registro %d offset %d\n", i, offset);
                // #endif
            }
        }

        #if DEBUG13
            fprintf(
                stderr, 
                "[%d) %d escrituras validadas en %s]\n",
                i + 1,
                info.nEscrituras,
                fichero_prueba);
        #endif

        memset(buffer, 0, sizeof(buffer));

        sprintf(buffer, "PID: %d\n", info.pid);
        sprintf(buffer + strlen(buffer), "Número de escrituras: %d\n", info.nEscrituras);
        sprintf(
            buffer + strlen(buffer), 
            "Primera Escritura\t%d\t%d\t%s\n",
            info.PrimeraEscritura.nEscritura,
            info.PrimeraEscritura.nRegistro,
            asctime(localtime(&info.PrimeraEscritura.fecha)));
        sprintf(
            buffer + strlen(buffer), 
            "Última escritura\t%d\t%d\t%s\n",
            info.UltimaEscritura.nEscritura,
            info.UltimaEscritura.nRegistro,
            asctime(localtime(&info.UltimaEscritura.fecha)));
        sprintf(
            buffer + strlen(buffer), 
            "Menor posicion\t%d\t%d\t%s\n",
            info.MenorPosicion.nEscritura,
            info.MenorPosicion.nRegistro,
            asctime(localtime(&info.MenorPosicion.fecha)));
        sprintf(
            buffer + strlen(buffer), 
            "Mayor posicion\t%d\t%d\t%s\n",
            info.MayorPosicion.nEscritura,
            info.MayorPosicion.nRegistro,
            asctime(localtime(&info.MayorPosicion.fecha)));
        sprintf(buffer + strlen(buffer), "\n\n");

        if((offset_informe += mi_write(fichero_informe, buffer, offset_informe, strlen(buffer)) < 0)) {
            fprintf(stderr, "verificacion.c: error escribiendo el el fichero %s", fichero_informe);
            bumount();
            return 1;
        }
    }

    bumount();
    return 0;
}
