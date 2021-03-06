/**
 * Autores: Arnau Vidal Moreno y Martín Ignacio Rizzo
 */ 
#include "directorios.h"

// 0 para la lectura, 1 para escritura
static struct UltimaEntrada MiniCacheEscrituraLectura[TAMCACHE];

/**
 * @brief dado un path o camino este se divide en dos, la ruta de la carpeta
 * o fichero y el propio nombre del fichero
 * 
 * @param camino    entrada que debemos cortar
 * @param inicial   Puntero al string que contendrá la parte inicial del camino
 * @param final     Puntero al string que contendrá el camino, menos inicial
 * @param tipo      Tipo de inodo (fichero o directorio)
 * @return f si es un fichero o d si es un directorio
 */
int extraer_camino(const char *camino,char *inicial,char *final,char *tipo){
    char* resto;
    
    if(camino[0] != '/') {
        return ERROR_EXIT;
    }

    // Localizamos la primera / después
    // de la inicial
    resto = strchr(camino + 1, '/');

    if(resto) {
        // Si resto no es NULL,
        // significa que hay segundo /
        strncpy(inicial, camino + 1, strlen(camino) - strlen(resto) - 1);
        strcpy(final, resto);
        strcpy(tipo, "d");

        return SUCCESS_EXIT;
    } else {
        strcpy(inicial, camino + 1);
        strcpy(final, "");
        strcpy(tipo, "f");

        return SUCCESS_EXIT;
    }

    // Si llega aquí es porque no es ni fichero ni
    // directorio; ha habido un error en algún sitio
    return ERROR_EXIT; 
}

/**
 * Función auxiliar para mostrar los mensajes de
 * error en función del código de error devuelto
 * por buscar_entrada()
 * 
 * @param error Código de error
 * 
 * @returns void
 */
void mostrar_error_buscar_entrada(int error) {
    switch(error) {
        case ERROR_CAMINO_INCORRECTO:
            fprintf(stderr, "Error: camino incorrecto\n");
            break;
        case ERROR_PERMISO_LECTURA:
            fprintf(stderr, "Error: permiso denegado de lectura\n");
            break;
        case ERROR_NO_EXISTE_ENTRADA_CONSULTA:
            fprintf(stderr, "Error: no existe el archivo o el directorio\n");
            break;
        case ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO:
            fprintf(stderr, "Error: no existe algún directorio intermedio\n");
            break;
        case ERROR_PERMISO_ESCRITURA:
            fprintf(stderr, "Error: permiso de escritura denegado\n");
            break;
        case ERROR_ENTRADA_YA_EXISTENTE:
            fprintf(stderr, "Error: el archivo ya existe\n");
            break;
        case ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO:
            fprintf(stderr, "Error: no es un directorio\n");
    }
}

/**
 * Busca una entrada determinada dentro de
 * los inodos. 
 * 
 * @param   camino_parcial  Cadena de caracteres que indica el camino
 * @param   p_inodo_dir     Número del inodo padre donde buscar la entrada
 * @param   p_inodo         Número del inodo hijo final
 * @param   p_entrada       Número de la entrada del directorio padre
 * @param   reservar        Bit para reservar, o no, una entrada de directorio,
 *                          si no existe
 * @param   permisos        Si se reserva un inodo, estos son los permisos que se 
 *                          le darán
 * 
 * @returns SUCCESS_EXIT si todo ha ido bien, 
 *          uno de los códigos de error (ver directorios.h)
 *          de lo contrario
 */
int buscar_entrada(
    const char* camino_parcial, 
    unsigned int* p_inodo_dir,
    unsigned int* p_inodo,
    unsigned int* p_entrada,
    char reservar,
    unsigned char permisos) {
    
    struct superbloque SB;
    struct entrada entrada;
    struct inodo inodo_dir;
    char inicial[sizeof(entrada.nombre)];
    char final[strlen(camino_parcial)];
    char tipo;
    int cant_entradas_inodo, 
    num_entrada_inodo;

    // Limpiamos todos los arrays, para no 
    // tener errores a la hora de comparalos
    memset(inicial, 0, sizeof(entrada.nombre));
    memset(final, 0, strlen(camino_parcial));
    memset(entrada.nombre, 0, sizeof(entrada.nombre));

    if(bread(posSB, &SB) == ERROR_EXIT) {
        #if DEBUG7
            fprintf(stderr, "[Error en buscar_entrada()]: no se ha podido leer el superbloque");
        #endif
        return ERROR_EXIT;
    }

    if(strcmp(camino_parcial, "/") == 0) {
        *p_inodo = SB.posInodoRaiz;
        *p_entrada = 0;
        return SUCCESS_EXIT;
    }

    if(extraer_camino(camino_parcial, inicial, final, &tipo) == ERROR_EXIT) {
        return ERROR_CAMINO_INCORRECTO;
    }

    #if DEBUG7
        fprintf(
            stderr, "[buscar_entrada() -> inicial: %s, final: %s, reservar: %d]\n", 
            inicial, 
            final, 
            reservar);
    #endif

    if(leer_inodo(*p_inodo_dir, &inodo_dir) == ERROR_EXIT) {
        #if DEBUG7
            fprintf(stderr, "[Error en bsucar_entrada()]: no se ha podido leer el inodo %d", *p_inodo_dir);
        #endif
        return ERROR_EXIT;
    }

    if((inodo_dir.permisos & 4) != 4) {
        return ERROR_PERMISO_LECTURA;
    }

    // Calculamos la cantidad de entradas que contiene el inodo
    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(struct entrada);
    num_entrada_inodo = 0;

    // Si el inodo tiene entradas, tenemos que recorrerlas
    // secuencialmente
    if(cant_entradas_inodo > 0) {
        if(mi_read_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) == ERROR_EXIT) {
            #if DEBUG7
                fprintf(stderr, "[Error en buscar_entrada()]: no se han podido leer la entrada %d del inodo %d\n", num_entrada_inodo, *p_inodo_dir);
            #endif
            return ERROR_EXIT;
        }
    
        while((num_entrada_inodo < cant_entradas_inodo) && strcmp(inicial, entrada.nombre) != 0) {
            num_entrada_inodo++;

            memset(entrada.nombre, 0, sizeof(entrada.nombre));
            if(mi_read_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) == ERROR_EXIT) {
                #if DEBUG7
                    fprintf(stderr, "[Error en buscar_entrada()]: no se han podido leer la entrada %d del inodo %d\n", num_entrada_inodo, *p_inodo_dir);
                #endif
                return ERROR_EXIT;
            }
        }
    }

    if(strcmp(inicial, entrada.nombre) != 0) {
        switch(reservar) {
            case 0:
                return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
            case 1:
                if(inodo_dir.tipo == 'f') return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;

                if((inodo_dir.permisos & 2) != 2) return ERROR_PERMISO_ESCRITURA;

                strcpy(entrada.nombre, inicial);

                if(tipo == 'd') {
                    if(strcmp(final, "/") == 0) {
                        entrada.ninodo = reservar_inodo('d', permisos);

                        #if DEBUG7
                            fprintf(stderr, "[buscar_entrada() -> reservado inodo %d de tipo %c y con permisos %d\n", entrada.ninodo, tipo, permisos);
                        #endif
                    } else {
                        return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                    }
                } else {
                    entrada.ninodo = reservar_inodo('f', permisos);

                    #if DEBUG7
                        fprintf(stderr, "[buscar_entrada() -> reservado inodo %d de tipo %c y con permisos %d\n", entrada.ninodo, tipo, permisos);
                    #endif

                }

                #if DEBUG7
                    fprintf(stderr, "[buscar_entrada() -> creada entrada %s con ninodo %d\n", inicial, entrada.ninodo);
                #endif

                if(mi_write_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) == ERROR_EXIT) {
                    #if DEBUG7
                        fprintf(stderr, "[Error en buscar_entrada()]: no se ha podido escribir la entrada en el inodo %d\n", *p_inodo_dir);
                    #endif
                    if(entrada.ninodo != -1) {
                        liberar_inodo(entrada.ninodo);
                        #if DEBUG7
                            fprintf(stderr, "buscar_entrada() -> liberando inodo %d", entrada.ninodo);
                        #endif
                    }
                    return ERROR_EXIT;
                }
        }
    }

    if(strcmp(final, "/") == 0 || strcmp(final, "") == 0) {
        if((num_entrada_inodo < cant_entradas_inodo) && (reservar == 1)) return ERROR_ENTRADA_YA_EXISTENTE;

        *p_inodo = entrada.ninodo;
        *p_entrada = num_entrada_inodo;
        
        return SUCCESS_EXIT;
    } else {
        *p_inodo_dir = entrada.ninodo;

        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
    }

    return SUCCESS_EXIT;
}

/**
 * Crea un fichero/directorio y su entrada
 * 
 * @param   camino      Camino donde crear el fichero/directorio
 * @param   permisos    Permisos con los que crear el fichero/directorio
 * 
 * @returns SUCCESS_EXIT si todo ha salido bien
 *          ERROR_EXIT de lo contrario
 */
int mi_creat(const char* camino, unsigned char permisos) {
    unsigned int p_inodo;
    unsigned int p_entrada;
    unsigned int p_inodo_dir = 0;
    int error;

    mi_waitSem();

    if((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos)) < 0) {
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return ERROR_EXIT;
    }

    mi_signalSem();
    return SUCCESS_EXIT;
}

/**
 * Muestra información sobre un directorio o
 * fichero determinado
 * 
 * @param   camino  Camino del directorio o fichero
 * @param   buffer  Buffer donde se introducirá la información
 *                  del fichero/directorio
 * 
 * @returns ERROR_EXIT si algo ha ido mal,
 *          el número total de entradas que tiene 
 *          el inodo de lo contrario
 */
int mi_dir(const char* camino, char* buffer, char tipo) {
    struct inodo inodo;
    struct entrada entrada;
    unsigned int p_inodo;
    unsigned int p_entrada;
    unsigned int p_inodo_dir = 0;
    unsigned int totEntradasInodo;
    int error;

    if((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4)) < 0) {
        mostrar_error_buscar_entrada(error);
        return ERROR_EXIT;
    }

    if(leer_inodo(p_inodo, &inodo) == ERROR_EXIT) {
        #if DEBUG8
            fprintf(stderr, "[Error en mi_dir()]: no se ha podido leer el inodo %d\n", p_entrada);
        #endif
        return ERROR_EXIT;
    }

    if((inodo.permisos & 4) != 4) {
        #if DEBUG8
            fprintf(stderr, "[Error en mi_dir()]: el inodo no tiene permisos de lectura\n");
        #endif
        return ERROR_EXIT;
    }

    if(inodo.tipo != tipo) {
        #if DEBUG8
            fprintf(stderr, "[Error en mi_dir()]: la sintaxis no concuerda con el tipo");
        #endif
        return ERROR_EXIT;
    }

    totEntradasInodo = inodo.tamEnBytesLog / sizeof(struct entrada);

    // Si el inodo es un directorio, mostramos sus
    // entradas
    if(inodo.tipo == 'd') {
        // Utilizamos un buffer para no tener que
        // leer entrada a entrada
        // struct entrada entradas[totEntradasBloque];
        // memset(entradas, 0, sizeof(entradas));

        int nEntrada = 0;

        mi_read_f(p_inodo, &entrada, nEntrada * sizeof(struct entrada), sizeof(struct entrada));

        for(int i = 0; i < totEntradasInodo; i++) {
            // Leemos el inodo correspondiente a cada entrada
            if(leer_inodo(entrada.ninodo, &inodo) == ERROR_EXIT) {
                #if DEBUG8
                    fprintf(stderr, "[Error en buscar_entrada()]: no se ha podido leer el inodo %d",
                    entrada.ninodo);
                #endif
                return ERROR_EXIT;
            }

            // Si el inodo es de un directorio,
            // lo mostramos en azul
            if(inodo.tipo == 'd') strcat(buffer, DIRECTORY_COLOR);
            // Si es un fichero ejecutable, lo mostramos
            // en verde y en negrita
            else if((inodo.permisos & 1) == 1) strcat(buffer, BOLD_GREEN);

            
            // Vamos creando el buffer, primero con
            // el nombre de la entrada
            strcat(buffer, entrada.nombre);
            strcat(buffer, "\t");

            strcat(buffer, RESET_COLOR);

            // // A continuación, el tipo
            strcat(buffer, inodo.tipo == 'd' ? "d" : "f");
            strcat(buffer, "\t");

            // Luego los permisos 
            strcat(buffer, (inodo.permisos & 4) == 4 ? "r" : "-");
            strcat(buffer, (inodo.permisos & 2) == 2 ? "w" : "-");
            strcat(buffer, (inodo.permisos & 1) == 1 ? "x" : "-");
            strcat(buffer, "\t\t");

            // Utilizamos otro buffer auxiliar para
            // poder concatenar el tamanyo del inodo
            char stringTamanyo[sizeof(inodo.tamEnBytesLog)];
            sprintf(stringTamanyo, "%d", inodo.tamEnBytesLog);

            strcat(buffer, stringTamanyo);
            strcat(buffer, "\t");

            // Y el mtime
            struct tm* tm;
            char tmp[80];

            tm = localtime(&inodo.mtime);
            sprintf(
                tmp, 
                "%d-%02d-%02d %02d:%02d:%02d", 
                tm->tm_year + 1900, 
                tm->tm_mon + 1, 
                tm->tm_mday, 
                tm->tm_hour, 
                tm->tm_min, 
                tm->tm_sec);
            strcat(buffer, tmp);

            strcat(buffer, "\n");
            
            mi_read_f(p_inodo, &entrada, ++nEntrada * sizeof(struct entrada), sizeof(struct entrada));
        }

        return totEntradasInodo;

    // Si el inodo es de un fichero, mostramos
    // únicamente el inodo
    } else if(inodo.tipo == 'f') {
        if(mi_read_f(p_inodo_dir, &entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada)) == ERROR_EXIT) {
            #if DEBUG8
                fprintf(stderr, 
                "[Error en mi_dir()]: no se ha podido leer la entrada %d del inodo %d",
                p_entrada,
                p_inodo);
            #endif
            return ERROR_EXIT;
        }

        if(leer_inodo(entrada.ninodo, &inodo) == ERROR_EXIT) {
            #if DEBUG8
                fprintf(stderr, "[Error en mi_dir()]: no se ha podido leer el inodo de la entrada %d",
                p_entrada);
            #endif
            return ERROR_EXIT;
        }

        if((inodo.permisos & 1) == 1) strcat(buffer, BOLD_GREEN);

        // Vamos creando el buffer, primero con
        // el nombre de la entrada
        strcat(buffer, entrada.nombre);
        strcat(buffer, "\t");

        strcat(buffer, RESET_COLOR);

        // // A continuación, el tipo
        strcat(buffer, "\t");

        // Luego los permisos 
        strcat(buffer, (inodo.permisos & 4) == 4 ? "r" : "-");
        strcat(buffer, (inodo.permisos & 2) == 2 ? "w" : "-");
        strcat(buffer, (inodo.permisos & 1) == 1 ? "x" : "-");
        strcat(buffer, "\t\t");

        // Utilizamos otro buffer auxiliar para
        // poder concatenar el tamanyo del inodo
        char stringTamanyo[sizeof(inodo.tamEnBytesLog)];
        sprintf(stringTamanyo, "%d", inodo.tamEnBytesLog);

        strcat(buffer, stringTamanyo);
        strcat(buffer, "\t");

        // Y el mtime
        struct tm* tm;
        char tmp[80];

        tm = localtime(&inodo.mtime);
        sprintf(
            tmp, 
            "%d-%02d-%02d %02d:%02d:%02d", 
            tm->tm_year + 1900, 
            tm->tm_mon + 1, 
            tm->tm_mday, 
            tm->tm_hour, 
            tm->tm_min, 
            tm->tm_sec);
        strcat(buffer, tmp);

        strcat(buffer, "\n");

        return SUCCESS_EXIT;
    }

    return ERROR_EXIT;
}

/**
 * Cambia los permisos de un fichero/directorio
 * 
 * @param   camino      Camino del directorio o fichero
 * @param   permisos    Nuevos permisos a poner en el fichero
 * 
 * @returns SUCCESS_EXIT si todo ha salido bien,
 *          ERROR_EXIT de lo contrario
 */
int mi_chmod(const char* camino, unsigned char permisos) {
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo;
    unsigned int p_entrada;
    int error;

    error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, permisos);
    if(error < 0) {
        mostrar_error_buscar_entrada(error);
        return ERROR_EXIT;
    }

    mi_chmod_f(p_inodo, permisos);

    return SUCCESS_EXIT;
}

/**
 * Deja en una estructura de tipo STAT
 * los datos sobre un inodo
 * 
 * @param   camino  Camino del directorio o fichero
 * @param   p_stat  STAT donde volcar los datos del inodo
 * 
 */
int mi_stat(const char* camino, struct STAT* p_stat) {
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo;
    unsigned int p_entrada;
    int error;

    error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
    if(error < 0) {
        mostrar_error_buscar_entrada(error);
        return ERROR_EXIT;
    }

    if(mi_stat_f(p_inodo, p_stat) == ERROR_EXIT) {
        return ERROR_EXIT;
    }

    return p_inodo;
}

/**
 * Escribe una cierta cantidad de bytes de
 * contenido en un fichero, empezando desde un offset
 * 
 * @param   camino  Camino del fichero donde escribir
 * @param   buff    Buffer con los datos a escribir
 * @param   offset  Offset desde donde empezar a escribir
 * @param   nbytes  Cantidad de bytes a escribir
 * 
 * @returns La cantidad de bytes escritos si todo ha salido bien,
 *          ERROR_EXIT de lo contrario 
 */
int mi_write(
    const char* camino, 
    const void* buff, 
    unsigned int offset, 
    unsigned int nbytes) {

    unsigned int p_inodo;
    unsigned int p_inodo_dir = 0;
    unsigned int p_entrada;
    int error;
    int bytesEscritos;

    // Si el camino termina con /, se trata
    // de un directorio, no de un fichero
    // if(*(camino + strlen(camino) - 1) == '/') {
    //     fprintf(stderr, "[Error en mi_write()]: el camino introducido no es de un fichero");
    //     return ERROR_EXIT;
    // }
    if(strcmp(MiniCacheEscrituraLectura->camino, camino) == 0) {
        p_inodo = MiniCacheEscrituraLectura[POS_CACHE_ESCRITURA].p_inodo;
    } else {
        error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6);
        strcpy(MiniCacheEscrituraLectura[POS_CACHE_ESCRITURA].camino, camino);
        MiniCacheEscrituraLectura[POS_CACHE_ESCRITURA].p_inodo= p_inodo; 
    }


    if(p_inodo < 0) {
        mostrar_error_buscar_entrada(error);
        return ERROR_EXIT;
    }

    bytesEscritos = mi_write_f(p_inodo, buff, offset, nbytes);
    if(bytesEscritos == ERROR_EXIT) {
        #if DEBUG9
            fprintf(stderr, "[Error en mi_write()]: no se ha podido escribir en el inodo %d", p_inodo);
        #endif
        return ERROR_EXIT;
    }

    return bytesEscritos;
}

/**
 * Lee una cierta cantidad de bytes de un
 * fichero determinado, a partir de un offset
 * 
 * @param   camino  Camino del fichero a leer
 * @param   buff    Buffer donde se volcarán los datos leídos
 * @param   offset  Offset desde donde empezar a leer
 * @param   nbytes  Cantidad de bytes a leer
 * 
 * @returns La cantidad de bytes leídos si todo ha ido bien,
 *          ERROR_EXIT de lo contario
 * 
 */
int mi_read(
    const char* camino, 
    void* buff, 
    unsigned int offset, 
    unsigned int nbytes) {

    unsigned int p_inodo;
    unsigned int p_inodo_dir = 0;
    unsigned int p_entrada;
    int error;
    int bytesLeidos;

    if(strcmp(MiniCacheEscrituraLectura->camino, camino) == 0) {
        p_inodo = MiniCacheEscrituraLectura[POS_CACHE_LECTURA].p_inodo;
    } else {
        error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6);
        strcpy(MiniCacheEscrituraLectura[POS_CACHE_LECTURA].camino, camino);
        MiniCacheEscrituraLectura[POS_CACHE_LECTURA].p_inodo= p_inodo;
        
    }


    if(p_inodo < 0) {
        mostrar_error_buscar_entrada(error);
        return ERROR_EXIT;
    }

    bytesLeidos = mi_read_f(p_inodo, buff, offset, nbytes);
    if(bytesLeidos == ERROR_EXIT) {
        #if DEBUG9
            fprintf(stderr, "[Error en mi_read()]: no se ha podido leer el inodo %d", p_inodo);
        #endif
        return ERROR_EXIT;
    }

    return bytesLeidos;
}

/**
 * Crea un enlace físico entre dos ficheros
 * 
 * @param   camino1 Camino que queremos enlazar
 * @param   camino2 Ruta del camino del enlace
 * 
 * @returns SUCCESS_EXIT si todo ha salido bien,
 *          ERROR_EXIT de lo contrario
 */
int mi_link(const char* camino1, const char* camino2) {
    struct inodo inodo;
    struct entrada entrada;
    int error;
    unsigned int p_inodo_dir1, p_inodo_dir2 = 0;
    unsigned int p_inodo1, p_inodo2;
    unsigned int p_entrada1, p_entrada2;

    mi_waitSem();

    if((error = buscar_entrada(camino1, &p_inodo_dir1, &p_inodo1, &p_entrada1, 0, 4)) < 0) {
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return ERROR_EXIT;
    }

    if(leer_inodo(p_inodo1, &inodo) == ERROR_EXIT) {
        #if DEBUG10
            fprintf(stderr, "[Error en mi_link()]: no se ha podido leer el inodo %d\n", p_inodo1);
        #endif
        mi_signalSem();
        return ERROR_EXIT;
    }

    if((inodo.permisos & 4) != 4) {
        #if DEBUG10
            fprintf(stderr, "[Error en mi_link()]: el inodo no tiene permisos de lectura\n");
        #endif
        mi_signalSem();
        return ERROR_EXIT; 
    }

    if((error = buscar_entrada(camino2, &p_inodo_dir2, &p_inodo2, &p_entrada2, 1, 6)) < 0) {
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return ERROR_EXIT;
    }

    if(mi_read_f(p_inodo_dir2, &entrada, p_entrada2 * sizeof(struct entrada), sizeof(struct entrada)) == ERROR_EXIT) {
        #if DEBUG10
            fprintf(stderr, 
            "[Error en mi_link()]: no se ha podido leer la entrada %d del inodo %d\n",
            p_entrada2,
            p_inodo_dir2);
        #endif
        mi_signalSem();
        return ERROR_EXIT; 
    }

    // Actualizamos el inodo al que apunta
    // la entrada y lo escribimos
    entrada.ninodo = p_inodo1;
    if(mi_write_f(p_inodo_dir2, &entrada, p_entrada2 * sizeof(struct entrada), sizeof(struct entrada)) == ERROR_EXIT) {
        #if DEBUG10
            fprintf(stderr, 
            "[Error en mi_link()]: no se ha podido escribir la entrada %d del inodo %d\n",
            p_entrada2,
            p_inodo_dir2);
        #endif
        mi_signalSem();
        return ERROR_EXIT; 
    }

    // El nuevo inodo creado para camino2 no es 
    // necesario, por tanto lo liberamos
    if(liberar_inodo(p_inodo2) == ERROR_EXIT) {
        #if DEBUG10
            fprintf(stderr, 
            "[Error en mi_link()]: no se ha podido liberar el inodo %d\n",
            p_inodo2);
        #endif
        mi_signalSem();
        return ERROR_EXIT; 
    }

    // Se trata de un enlace, por tanto 
    // aumentamos el número de enlaces
    // del inodo del camino1
    inodo.nlinks++;
    inodo.ctime = time(NULL);

    if(escribir_inodo(p_inodo1, &inodo) == ERROR_EXIT) {
        #if DEBUG10
            fprintf(stderr, 
            "[Error en mi_link()]: no se ha podido escribir el inodo %d\n",
            p_inodo1);
        #endif
        mi_signalSem();
        return ERROR_EXIT; 
    }

    mi_signalSem();
    return SUCCESS_EXIT;
}

/**
 * Borra la entrada de directorio especificada.
 * Si es el último enlace existente, borra el inodo
 * del fichero/directorio.
 * 
 * @param   camino  Camino de la entrada
 * 
 * @returns SUCCESS_EXIT si todo ha salido bien,
 *          ERROR_EXIT de lo contrario
 */
int mi_unlink(const char* camino) {
    struct inodo inodo;
    struct entrada entrada;
    unsigned int p_inodo;
    unsigned int p_inodo_dir = 0;
    unsigned int p_entrada;
    unsigned int num_entradas_dir;
    int error;

    mi_waitSem();

    if((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6)) < 0) {
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return ERROR_EXIT;
    }

    if(leer_inodo(p_inodo, &inodo) == ERROR_EXIT) {
        #if DEBUG10
            fprintf(stderr, "[Error en mi_unlink()]: no se ha podido leer el inodo %d\n", p_inodo);
        #endif
        mi_signalSem();
        return ERROR_EXIT;
    }

    if(inodo.tipo == 'd' && inodo.tamEnBytesLog > 0) {
        #if DEBUG10
            fprintf(stderr, "[Error en mi_unlink()]: no se puede eliminar un directorio que no esté vacío\n");
        #endif
        fprintf(stderr, "El directorio %s no está vacío", camino);
        mi_signalSem();
        return ERROR_EXIT;
    }

    if(leer_inodo(p_inodo_dir, &inodo) == ERROR_EXIT) {
        #if DEBUG10
            fprintf(
                stderr, 
                "[Error en mi_unlink()]: no se ha podido leer el inodo %d, padre de %d\n", 
                p_inodo_dir, 
                p_inodo);
        #endif
        mi_signalSem();
        return ERROR_EXIT;
    }

    num_entradas_dir = inodo.tamEnBytesLog/sizeof(struct entrada);

    if(p_entrada != num_entradas_dir - 1) {

        // Leemos la última entrada
        if(mi_read_f(p_inodo_dir, &entrada, (num_entradas_dir - 1) * sizeof(struct entrada), sizeof(entrada)) == ERROR_EXIT) {
            #if DEBUG10
                fprintf(stderr, "[Error en mi_unlink()]: no se ha podido leer la entrada %d\n", num_entradas_dir - 1);
            #endif
            mi_signalSem();
            return ERROR_EXIT;
        }

        // Escribimos la entrada leída en el
        // sitio en el que estaba la entrada a eliminar
        if(mi_write_f(p_inodo_dir, &entrada, p_entrada * sizeof(struct entrada), sizeof(entrada)) == ERROR_EXIT) {
            #if DEBUG10
                fprintf(
                    stderr, 
                    "[Error en mi_unlink()]: no se ha escribir la entrada %d en la posición de p_entrada %d\n", 
                    num_entradas_dir,
                    p_entrada);
            #endif
            mi_signalSem();
            return ERROR_EXIT;
        }
    }

    // Truncamos la última parte del inodo
    // que contiene la entrada
    if(mi_truncar_f(p_inodo_dir, inodo.tamEnBytesLog - sizeof(struct entrada)) == ERROR_EXIT) {
        #if DEBUG10
            fprintf(
                stderr, 
                "[Error en mi_unlink()]: no se ha podido truncar el inodo %d\n", 
                p_inodo_dir);
        #endif
        mi_signalSem();
        return ERROR_EXIT;
    }

    if(leer_inodo(p_inodo, &inodo) == ERROR_EXIT) {
        #if DEBUG10
            fprintf(
                stderr, 
                "[Error en mi_unlink()]: no se ha podido leer el inodo %d de la entrada %d\n", 
                p_inodo,
                p_entrada);
        #endif
        mi_signalSem();
        return ERROR_EXIT;
    }

    inodo.nlinks--;

    if(inodo.nlinks == 0) {
       // Si ya no quedan enlaces, podemos
       // eliminar el inodo
       if(liberar_inodo(p_inodo) == ERROR_EXIT) {
           #if DEBUG10
                fprintf(
                    stderr, 
                    "[Error en mi_unlink()]: no se ha podido liberar elinodo %d\n", 
                    p_inodo);
            #endif
            mi_signalSem();
            return ERROR_EXIT;
       }
    }

    inodo.ctime = time(NULL);
    if(escribir_inodo(p_inodo, &inodo) == ERROR_EXIT) {
        #if DEBUG10
            fprintf(
                stderr, 
                "[Error en mi_unlink()]: no se ha podido escribir el inodo %d", 
                p_inodo);
        #endif
        mi_signalSem();
        return ERROR_EXIT;
    }

    mi_signalSem();
    return SUCCESS_EXIT;
}