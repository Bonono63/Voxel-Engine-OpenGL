INC_DIR = ./inc/
SRC_DIR = ./src/
LIB_DIR = ./lib/
INC_DIR += -I./glfw-3.4/include/
LIB_DIR += -L./glfw-3.4/bin/src/
INC_DIR += -I./cglm-0.9.4/include/
LIB_DIR += -L./cglm-0.9.4/build/

#GLFW's signature is different on Windows and linux for some reason
ifeq ($(OS), Windows_NT)
	LIBS = -lglfw3 -lcglm -lm
else
	LIBS = -lglfw -lcglm -lm
endif

CC=gcc
CFLAGS=-I$(INC_DIR) -Bstatic -L$(LIB_DIR) -O0 -g -Wall


engine : main.o gl.o io.o render.o voxel.o;
	$(CC) $(CFLAGS) bin/main.o bin/gl.o bin/io.o bin/render.o bin/voxel.o $(LIBS) -o bin/engine

main.o : $(SRC_DIR)/main.c ;
	$(CC) -c $(CFLAGS) $(SRC_DIR)/main.c -o bin/main.o

gl.o : $(SRC_DIR)/gl.c ;
	$(CC) -c $(CFLAGS) $(SRC_DIR)/gl.c -o bin/gl.o

io.o : $(SRC_DIR)/io.c ;
	$(CC) -c $(CFLAGS) $(SRC_DIR)/io.c -o bin/io.o

render.o : $(SRC_DIR)/render.c ;
	$(CC) -c $(CFLAGS) $(SRC_DIR)/render.c -o bin/render.o

voxel.o : $(SRC_DIR)/voxel.c ;
	$(CC) -c $(CFLAGS) $(SRC_DIR)/voxel.c -o bin/voxel.o

.PHONY: clean

clean:
	rm bin/*.o
