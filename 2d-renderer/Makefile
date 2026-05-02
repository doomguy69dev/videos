CC = cc
CC_FLAGS = -std=c99 -Wall
LD_FLAGS = -lglfw -lGLEW -lGL -lm

all: debug

debug:
	$(CC) $(CC_FLAGS) main.c -o out $(LD_FLAGS) -O0 -g
	./out

release:
	$(CC) $(CC_FLAGS) main.c -o out $(LD_FLAGS) -O3
