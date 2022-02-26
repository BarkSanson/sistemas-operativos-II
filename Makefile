CC=gcc
CFLAGS=-c -g -Wall -std=gnu99

SOURCES=mi_mkfs.c bloques.c
BUILD_DIR=build/
LIBRARIES=bloques.o
INCLUDES=bloques.h
PROGRAMS=mi_mkfs
OBJECTS=$(SOURCES:.c=.o)

all: $(OBJECTS) $(PROGRAMS)

$(PROGRAMS): $(BUILD_DIR)$(LIBRARIES) $(INCLUDES)
	$(CC) $(LDFLAGS) $(BUILD_DIR)$(LIBRARIES) $(BUILD_DIR)$@.o -o $(BUILD_DIR)$@

%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -o $(BUILD_DIR)$@ -c $<

.PHONY: clean
clean:
	rm -rf *.o *~ $(PROGRAMS) disco* ext*