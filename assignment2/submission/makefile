# List source and output directories here
SOURCES = $(wildcard *.c) $(wildcard include/*.c) 
OBJECTS = ${SOURCES:.c=.o}

CC=gcc
CFLAGS   := -Wall -D_GNU_SOURCE -g   # some warnings about bad code
LDFLAGS  := -Llib              # -L is a linker flag
LIBS   := -lrt -lz -lpthread   # Left empty if no libs are needed



all: web_server

web_server: ${OBJECTS}
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ ${OBJECTS} $(LIBS)

clean:
	-rm -f *.o include/*.o web_server

