CC = cc
CC_FLAGS = -std=c99 -Wall -lraylib -lm -lX11

all: debug

debug:
	$(CC) main.c -o main $(CC_FLAGS) -O0 -g -fsanitize=address
	./main

release:
	$(CC) main.c -o main $(CC_FLAGS) -O3
