CC=gcc
CFLAGS=-c -g -Wall -std=gnu99 -lm
LDFLAGS=-pthread

BUILD_DIR=build/
SOURCES=mi_mkfs.c bloques.c ficheros_basico.c ficheros.c directorios.c leer_sf.c escribir.c leer.c permitir.c truncar.c mi_ls.c mi_mkdir.c mi_chmod.c mi_stat.c mi_touch.c mi_cat.c mi_escribir.c mi_link.c mi_rm.c mi_rmdir.c semaforo_mutex_posix.c simulacion.c verificacion.c
LIBRARIES=$(BUILD_DIR)bloques.o $(BUILD_DIR)ficheros_basico.o $(BUILD_DIR)ficheros.o $(BUILD_DIR)directorios.o $(BUILD_DIR)semaforo_mutex_posix.o
INCLUDES=bloques.h ficheros_basico.h ficheros.h directorios.h semaforo_mutex_posix.h simulacion.h verificacion.h
PROGRAMS=mi_mkfs escribir leer leer_sf permitir truncar mi_ls mi_mkdir mi_chmod mi_stat mi_touch mi_escribir mi_cat mi_link mi_rm mi_rmdir simulacion verificacion
OBJECTS=$(SOURCES:.c=.o)

all: $(OBJECTS) $(PROGRAMS) 

$(PROGRAMS): $(LIBRARIES) $(INCLUDES)
	$(CC) $(LDFLAGS) $(LIBRARIES) $(BUILD_DIR)$@.o -lm -o $(BUILD_DIR)$@

%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -o $(BUILD_DIR)$@ -c $<

.PHONY: clean
clean:
	rm -rf *.o *~ disco* ext* $(BUILD_DIR)* scripts/ext*.txt scripts/disco