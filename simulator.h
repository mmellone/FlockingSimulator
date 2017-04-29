/* Flocking Simulator
 * Parallel Programming Final Project
 *
 * This project is a parallel discrete event simulation of flocking birds using
 * MPI and pThreads and is designed to run on the RPI BG/Q super computer AMOS
 *
 * Written by: Kevin O'Neill and Mitchell Mellone
 * Due: 2 May 2017
*/

/*
 * NOTES:
 * - The coordinate (0, 0) is the bottom left corner and
 *   (universe_size, universe_size) is the top right corner
*/
#ifndef _SIMULATOR_H_
#define _SIMULATOR_H_

/* Imports */
#include <math.h>
#include <mpi.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef BGQ
#include <hwi/include/bqc/A2_inlines.h>
#endif

/* Default values */
#define UNIVERSE_SIZE_DEFAULT 500
#define NUM_BIRDS_DEFUALT 128
#define NUM_ITERATIONS_DEFAULT 100
#define NUM_THREADS_DEFAULT 1
#define PRINTING 1 /* 1 enables printing, 0 disables printing */
#define BIRD_SPEED 10.0
#define NEIGHBOR_RADIUS 50.0

/* Constants */
#define DEG_TO_RAD (M_PI / 180.0)
#define BIRD_SIZE 13
#define CLOCK_RATE 1600000000

/* Struct Definitions */
typedef struct {
  pthread_mutex_t count_lock;
  pthread_cond_t ok_to_proceed;
  int count;
} my_pthread_barrier_t;

typedef struct Bird {
  int id;
  int x, y, z,
    dx, dy, dz;
  
  int next_x, next_y, next_z,
    next_dx, next_dy, next_dz;
} Bird;
MPI_Datatype MPI_Bird;  /* an MPI_Datatype for the Bird struct below */

/* Global Variables */
int universe_size; /* Width/height of the universe */
int num_birds; /* Number of birds to simulate */
int birds_per_rank; /* Number of birds simulated by each rank */
int birds_per_thread;
int max_time;
int num_threads; /* Number of threads per mpi process */
int commsize;  /* the number of MPI Ranks */
int commrank;  /* the MPI rank of this process */
Bird * birds; /* Local birds in this rank */
Bird * all_birds; /* All birds in simulation */
FILE * output_file;
my_pthread_barrier_t * pthread_barrier;


/* Cycle counters/timers for MPI communication vs. computation */
#ifdef BGQ
unsigned long long comm_time = 0;
unsigned long long comp_time = 0;
#else
double comm_time = 0.0,
  comp_time = 0.0;
#endif

/* Function Declarations */
void * run_simulation( void *start_bird_p );
void decide_next_move( Bird *birds, int bird_index, Bird *b );
void apply_next_move( Bird *b );
void print( FILE * fout, Bird *birds, int sim_time, int csv_format );
int read_cl_args( int * argc_p, char *** argv_p );
void print_help_msg( void );

double distance( Bird *b1, Bird* b2 );
void normalize( double *x, double *y, double *z, double length );

void my_pthread_init_barrier(my_pthread_barrier_t *b);
void my_pthread_barrier (my_pthread_barrier_t *b, int num_threads);

#endif
