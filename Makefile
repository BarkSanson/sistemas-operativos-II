CC=gcc
CFLAGS=-c -g -Wall -std=gnu99

SOURCES=mi_mkfs.c bloques.c ficheros_basico.c
BUILD_DIR=build/
LIBRARIES=$(BUILD_DIR)bloques.o $(BUILD_DIR)ficheros_basico.o
INCLUDES=bloques.h ficheros_basico.h
PROGRAMS=mi_mkfs
OBJECTS=$(SOURCES:.c=.o)

all: $(OBJECTS) $(PROGRAMS)

$(PROGRAMS): $(LIBRARIES) $(INCLUDES)
	$(CC) $(LDFLAGS) $(LIBRARIES) $(BUILD_DIR)$@.o -o $(BUILD_DIR)$@

%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -o $(BUILD_DIR)$@ -c $<

.PHONY: clean
clean:
	rm -rf *.o *~ $(PROGRAMS) disco* ext* $(BUILD_DIR)/*