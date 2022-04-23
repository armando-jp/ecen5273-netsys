# List source and output directories here
SOURCES_DFC = dfc.c $(wildcard include/*.c) 
OBJECTS_DFC = ${SOURCES_DFC:.c=.o}

SOURCES_DFS1 = dfs1.c $(wildcard include/*.c) 
OBJECTS_DFS1 = ${SOURCES_DFS1:.c=.o}

SOURCES_DFS2 = dfs2.c $(wildcard include/*.c) 
OBJECTS_DFS2 = ${SOURCES_DFS2:.c=.o}

SOURCES_DFS3 = dfs3.c $(wildcard include/*.c) 
OBJECTS_DFS3 = ${SOURCES_DFS3:.c=.o}

SOURCES_DFS4 = dfs4.c $(wildcard include/*.c) 
OBJECTS_DFS4 = ${SOURCES_DFS4:.c=.o}

CC=gcc
CFLAGS   := -Wall -D_GNU_SOURCE -g   # some warnings about bad code
LDFLAGS  := -Llib              # -L is a linker flag
LIBS   := -lrt -lz -lpthread   # Left empty if no libs are needed



all: dfc dfs1 dfs2 dfs3 dfs4

dfc: ${OBJECTS_DFC}
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ ${OBJECTS_DFC} $(LIBS)

dfs1: ${OBJECTS_DFS1}
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ ${OBJECTS_DFS1} $(LIBS)

dfs2: ${OBJECTS_DFS2}
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ ${OBJECTS_DFS2} $(LIBS)

dfs3: ${OBJECTS_DFS3}
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ ${OBJECTS_DFS3} $(LIBS)

dfs4: ${OBJECTS_DFS4}
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ ${OBJECTS_DFS4} $(LIBS)

clean:
	-rm -f *.o include/*.o dfc