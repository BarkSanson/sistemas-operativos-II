#include "ficheros.h"

#define TAMNOMBRE 60

#define DEBUG7 0

#define ERROR_CAMINO_INCORRECTO -2
#define ERROR_PERMISO_LECTURA -3
#define ERROR_NO_EXISTE_ENTRADA_CONSULTA -4
#define ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO -5
#define ERROR_PERMISO_ESCRITURA -6
#define ERROR_ENTRADA_YA_EXISTENTE -7
#define ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO -8

#define TAMFILA 100
#define TAMBUFFER (TAMFILA*1000)

struct entrada{
    char nombre[TAMNOMBRE];
    unsigned int ninodo;
};

int extraer_camino(
    const char *camino,
    char *inicial,
    char *final,
    char *tipo);
void mostrar_error_buscar_entrada(int error);
int buscar_entrada(
    const char* camino_parcial, 
    unsigned int* p_inodo_dir,
    unsigned int* p_inodo,
    unsigned int* p_entrada,
    char reservar,
    unsigned char permisos);

int mi_creat(const char* camino, unsigned char permisos);
int mi_dir(const char* camino, char* buffer);
int mi_chmod(const char* camino, unsigned char permisos);
int mi_stat(const char* camino, struct STAT* p_stat);
int mi_write(
    const char* camino, 
    const void* buff, 
    unsigned int offset, 
    unsigned int nbytes);
int mi_read(
    const char* camino, 
    void* buff, 
    unsigned int offset, 
    unsigned int nbytes);
