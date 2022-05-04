### Funcionamiento
Para compilar el proyecto, hay que utilizar el comando ```make``` en la raíz del mismo.
Los ejecutables necesarios (escribir, leer, etc.) estarán dentro de la carpeta build.
Los scripts están dentro de la carpeta scripts.
Debido a que los ejecutables están dentro de la carpeta build, los ficheros resultantes de
estos se crearán en este mismo directorio. Por ejemplo, cuando se utilice ```mi_mkfs```,
el dispositivo virtual se creará dentro de build.

### Observaciones
El proyecto en gran parte funciona de forma correcta. Sin embargo, somos conscientes de un
bug aparecido a última hora en el cual en el caso de utilizar ```./escribir <nombre_dispositivo> <"$(cat fichero)> 1```
se reservan los nodos de 9 en 9. Aunque hemos dedicado varias horas de debugging para intentar encontrar
cuál es la causa de este bug, no hemos sido capaces de hacerlo.