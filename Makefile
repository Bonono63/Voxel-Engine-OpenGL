INC_DIR = ./inc/
SRC_DIR = ./src/
LIB_DIR = ./lib/
INC_DIR += -I./glfw-3.4/include/
LIB_DIR += -L./glfw-3.4/bin/src/
INC_DIR += -I./cglm-0.9.4/include/
LIB_DIR += -L./cglm-0.9.4/build/

LIBS = -lglfw -lcglm -lm

CC=gcc
CFLAGS=-I$(INC_DIR) -L$(LIB_DIR) -O0 -g -Wall

engine : main.o gl.o io.o render.o ;
	$(CC) $(CFLAGS) main.o gl.o io.o render.o $(LIBS) -o engine


main.o : $(SRC_DIR)/main.c ;
	$(CC) -c $(CFLAGS) $(SRC_DIR)/main.c -o main.o

gl.o : $(SRC_DIR)/gl.c ;
	$(CC) -c $(CFLAGS) $(SRC_DIR)/gl.c -o gl.o

io.o : $(SRC_DIR)/io.c ;
	$(CC) -c $(CFLAGS) $(SRC_DIR)/io.c -o io.o

render.o : $(SRC_DIR)/render.c ;
	$(CC) -c $(CFLAGS) $(SRC_DIR)/render.c -o render.o
