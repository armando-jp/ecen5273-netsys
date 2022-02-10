# List source and output directories here
CLIENT_SRC_DIR := client
SERVER_SRC_DIR := server
INCLUDE_SRC_DIR := include
OBJ_DIR := obj
BIN_DIR := bin

# Name the final target(s)
EXE := $(BIN_DIR)/client
EXE += $(BIN_DIR)/server

# List your source files
SRC := $(wildcard $(CLIENT_SRC_DIR)/*.c)
SRC += $(wildcard $(SERVER_SRC_DIR)/*.c)
SRC += $(wildcard $(INCLUDE_SRC_DIR)/*.c)

# From the source files, list the object files
OBJ := $(patsubst $(CLIENT_SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))
OBJ += $(patsubst $(SERVER_SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))
OBJ += $(patsubst $(INCLUDE_SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))


CFLAGS   := -Wall -std=c99     # some warnings about bad code
LDFLAGS  := -Llib              # -L is a linker flag
LDLIBS   := -lrt -lz           # Left empty if no libs are needed

all: client server

client: ./client/uftp_client.c ./include/msg.c ./include/utils.c ./include/cli.c ./include/socket.c ./include/timer.c ./include/file.c ./include/crc.c ./include/packet.c 
	$(CC) -o client_out ./client/uftp_client.c  ./include/msg.c ./include/utils.c ./include/cli.c ./include/socket.c ./include/timer.c ./include/file.c ./include/crc.c ./include/packet.c $(LDLIBS) $(CFLAGS)

server: ./server/uftp_server.c ./include/msg.c ./include/utils.c ./include/cli.c ./include/socket.c ./include/timer.c ./include/file.c ./include/crc.c ./include/packet.c
	$(CC) -o server_out ./server/uftp_server.c  ./include/msg.c ./include/utils.c ./include/cli.c ./include/socket.c ./include/timer.c ./include/file.c ./include/crc.c ./include/packet.c $(LDLIBS) $(CFLAGS)
