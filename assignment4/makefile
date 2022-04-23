# List source and output directories here
SOURCES_DFC = dfc.c $(wildcard include/*.c) 
OBJECTS_DFC = ${SOURCES_DFC:.c=.o}

SOURCES_DFS1 = dfs1.c $(wildcard include/*.c) 
OBJECTS_DFS1 = ${SOURCES_DFS1:.c=.o}

SOURCES_DFS2 = dfs2.c $(wildcard include/*.c) 
OBJECTS_DFS2 = ${SOURCES_DFS2:.c=.o}

CC=gcc
CFLAGS   := -Wall -D_GNU_SOURCE -g   # some warnings about bad code
LDFLAGS  := -Llib              # -L is a linker flag
LIBS   := -lrt -lz -lpthread   # Left empty if no libs are needed



all: dfc dfs1 dfs2

dfc: ${OBJECTS_DFC}
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ ${OBJECTS_DFC} $(LIBS)

dfs1: ${OBJECTS_DFS1}
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ ${OBJECTS_DFS1} $(LIBS)

dfs2: ${OBJECTS_DFS2}
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ ${OBJECTS_DFS2} $(LIBS)

clean:
	-rm -f *.o include/*.o dfc