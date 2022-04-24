CC=gcc
CFLAGS=-c -g -Wall -std=gnu99 -lm

BUILD_DIR=build/
SOURCES=mi_mkfs.c bloques.c ficheros_basico.c ficheros.c leer_sf.c escribir.c leer.c permitir.c
LIBRARIES=$(BUILD_DIR)bloques.o $(BUILD_DIR)ficheros_basico.o $(BUILD_DIR)ficheros.o
INCLUDES=bloques.h ficheros_basico.h ficheros.h
PROGRAMS=mi_mkfs escribir leer leer_sf permitir
OBJECTS=$(SOURCES:.c=.o)

all: $(OBJECTS) $(PROGRAMS) 

$(PROGRAMS): $(LIBRARIES) $(INCLUDES)
	$(CC) $(LDFLAGS) $(LIBRARIES) $(BUILD_DIR)$@.o -lm -o $(BUILD_DIR)$@

%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -o $(BUILD_DIR)$@ -c $<

.PHONY: clean
clean:
	rm -rf *.o *~ disco* ext* $(BUILD_DIR)*