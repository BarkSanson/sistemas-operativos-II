# Funcionamiento
Para compilar el proyecto, hay que utilizar el comando ```make``` en la raíz del mismo.
Los ejecutables necesarios (escribir, leer, etc.) estarán dentro de la carpeta build.
Los scripts están dentro de la carpeta scripts.

# Mejoras
A continuación se describen las mejoras realizadas al sistema de ficheros:
- **initMB()**: se ha optimizado la función para que reserve directamente
los bloques pertenecientes al superbloque y al propio mapa de bits.
- **mi_touch**: se ha creado el programa mi_touch, que se encarga de crear únicamente
ficheros. Para crear directorios se utilizará el programa mi_mkdir.
- **mi_rmdir**: se ha creado el programa mi_rmdir, que se encarga de eliminar
directorios no vacíos. Para eliminar ficheros, existe el programa mi_rm.
- **mi_dir()**: se han realizado tres mejoras a la función mi_dir():
    - Se ha permitido que la función admita también ficheros, no 
    únicamente directorios
    - No solo lista el nombre del fichero/directorio, sino también su tipo,
    sus permisos, su mtime y su tamEnBytesLog.
    - Utiliza colores para diferenciar entre directorios, ficheros y
    ficheros ejecutables.