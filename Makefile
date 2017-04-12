all: simulator.c
	mpicc -I. -O3 simulator.c -o simulator -lm
