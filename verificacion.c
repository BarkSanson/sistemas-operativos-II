/**
 * Autores: Arnau Vidal Moreno y Martín Ignacio Rizzo
 */ 

#include "verificacion.h"

int main(int argc, char** argv) {
    struct STAT stat;
    struct entrada entradas[NUMPROCESOS];
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
        fprintf(stderr, "verificacion.c: no se ha podido leer el stat de %s", directorio_simulacion);
        return 1;
    }

    numEntradas = stat.tamEnBytesLog / sizeof(struct entrada);

    if(numEntradas != NUMPROCESOS) {
        fprintf(stderr, "verificacion.c: el número de entradas no coincide con el de procesos");
        return 1;
    }

    // Creamos el fichero informe.txt
    char fichero_informe[
        strlen(directorio_simulacion) + strlen("informe.txt") + 1];

    sprintf(fichero_informe, "%sinforme.txt", directorio_simulacion);

    if(mi_creat(fichero_informe, 6) == ERROR_EXIT) {
        fprintf(stderr, "verificacion.c: no se ha podido crear el fichero %s", fichero_informe);
        return 1;
    }

    // Leemos las entradas del directorio de la simulación
    if(mi_read(directorio_simulacion, entradas, 0, sizeof(entradas) * NUMPROCESOS) == ERROR_EXIT) {
        fprintf(stderr, "verificacion.c: error leyendo las entradas de %s", directorio_simulacion);
        return 1;
    }

    for(int i = 0; i < numEntradas; i++) {
        pid_t pid;
        struct INFORMACION info;
        char fichero_prueba[
            strlen(directorio_simulacion) + strlen(entradas[i].nombre) + strlen("prueba.dat") + 4
        ];
        char buffer[BLOCKSIZE];
        int cant_registros_buffer_escrituras = 256;
        struct REGISTRO buffer_escrituras[cant_registros_buffer_escrituras];
        int offset_escrituras = 0;
        int offset_informe;
        int leidos;

        pid = atoi(strchr(entradas[i].nombre, '_') + 1);
        info.pid = pid;
        info.nEscrituras = 0;

        sprintf(
            fichero_prueba, 
            "%s%s/prueba.dat",
            directorio_simulacion,
            entradas[i].nombre);

        memset(buffer_escrituras, 0, sizeof(buffer_escrituras));

        // Recorremos secuencialmente los registros escritos en prueba.dat
        offset_escrituras = 0;
        while((leidos = mi_read(fichero_prueba, buffer_escrituras, offset_escrituras, sizeof(buffer_escrituras))) > 0) {
            for(int j = 0; j < cant_registros_buffer_escrituras; j++) {
                if(info.pid == buffer_escrituras[j].pid) {
                    if(info.nEscrituras == 0) {
                        // Inicializamos todo con el primer registro
                        info.PrimeraEscritura = buffer_escrituras[j];
                        info.UltimaEscritura = buffer_escrituras[j];
                        info.MenorPosicion = buffer_escrituras[j];
                        info.MayorPosicion = buffer_escrituras[j];
                        info.nEscrituras++;
                    } else {
                        // Comparamos el registro con el guardado en info.
                        // Si se ha escrito antes/despues y su nEscritura es menor/mayor,
                        // entonces lo sustituimos en la Primera/UltimaEscritura
                        if((difftime(buffer_escrituras[j].fecha, info.UltimaEscritura.fecha)) >= 0 &&
                            buffer_escrituras[j].nEscritura > info.UltimaEscritura.nEscritura) {
                            info.UltimaEscritura = buffer_escrituras[j];
                        }

                        if((difftime(buffer_escrituras[j].fecha, info.PrimeraEscritura.fecha)) <= 0 &&
                            buffer_escrituras[j].nEscritura < info.PrimeraEscritura.nEscritura) {
                            info.PrimeraEscritura = buffer_escrituras[j];
                        }
                        
                        // Con Mayor/MenorPosicion, simplemente compramos el nRegistro
                        if(buffer_escrituras[j].nRegistro > info.MayorPosicion.nRegistro) {
                            info.MayorPosicion = buffer_escrituras[j];
                        }

                        if(buffer_escrituras[j].nRegistro < info.MenorPosicion.nRegistro) {
                            info.MenorPosicion = buffer_escrituras[j];
                        }
                        info.nEscrituras++;
                    }
                }
            }
            memset(buffer_escrituras, 0, sizeof(buffer_escrituras));
            offset_escrituras += sizeof(buffer_escrituras);
        }

        #if DEBUG13
            fprintf(
                stderr,
                "[%d) %d escrituraas validadas en %s]\n",
                i + 1,
                info.nEscrituras,
                fichero_prueba
            );
        #endif

        // Resetamos el buffer y le escribimos la información relevante
        memset(buffer, 0, BLOCKSIZE);

        sprintf(buffer, "PID: %d\n", info.pid);
        sprintf(buffer + strlen(buffer), "Número de escrituras: %d\n", info.nEscrituras);
        sprintf(
            buffer + strlen(buffer), 
            "Primera escritura %d %d %s\n",
            info.PrimeraEscritura.nEscritura,
            info.PrimeraEscritura.nRegistro,
            asctime(localtime(&info.PrimeraEscritura.fecha)));
        sprintf(
            buffer + strlen(buffer), 
            "Ultima escritura %d %d %s\n",
            info.UltimaEscritura.nEscritura,
            info.UltimaEscritura.nRegistro,
            asctime(localtime(&info.UltimaEscritura.fecha)));
        sprintf(
            buffer + strlen(buffer), 
            "Menor posición %d %d %s\n",
            info.MenorPosicion.nEscritura,
            info.MenorPosicion.nRegistro,
            asctime(localtime(&info.MenorPosicion.fecha)));
        sprintf(
            buffer + strlen(buffer), 
            "Mayor posición %d %d %s\n",
            info.MayorPosicion.nEscritura,
            info.MayorPosicion.nRegistro,
            asctime(localtime(&info.MayorPosicion.fecha)));

        sprintf(buffer + strlen(buffer), "\n");

        offset_informe += mi_write(fichero_informe, buffer, offset_informe, strlen(buffer)); 
    }

    bumount();
    return 0;
}
