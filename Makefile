CC=gcc
CFLAGS=-c -g -Wall -std=gnu99 -lm

SOURCES=mi_mkfs.c bloques.c ficheros_basico.c ficheros.c
BUILD_DIR=build/
LIBRARIES=$(BUILD_DIR)bloques.o $(BUILD_DIR)ficheros_basico.o $(BUILD_DIR)ficheros.o
INCLUDES=bloques.h ficheros_basico.h ficheros.h
TESTS_DIR=tests/
PROGRAMS=mi_mkfs 
TESTS_SOURCES=leer_sf.c escribir.c leer.c permitir.c
TESTS_PROGRAMS=leer_sf escribir leer permitir
OBJECTS=$(SOURCES:.c=.o)

all: $(OBJECTS) $(PROGRAMS)

$(PROGRAMS): $(LIBRARIES) $(INCLUDES)
	$(CC) $(LDFLAGS)  $(LIBRARIES) $(BUILD_DIR)$@.o -lm -o $(BUILD_DIR)$@

%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -o $(BUILD_DIR)$@ -c $<

tests:	 

.PHONY: clean
clean:
	rm -rf *.o *~ $(PROGRAMS) $(TESTS_DIR)$(TESTS_PROGRAMS) disco* ext* $(BUILD_DIR)/*