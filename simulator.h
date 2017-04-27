/* Flocking Simulator
 * Parallel Programming Final Project
 *
 * This project is a parallel discrete event simulation of flocking birds using
 * MPI and pThreads and is designed to run on the RPI BG/Q super computer AMOS
 *
 * Written by: Kevin O'Neill and Mitchell Mellone
 * Due: 2 May 2017
*/

/* NOTES:
 * - The coordinate (0, 0) is the top left corner and
 *   (universe_size, universe_size) is the bottom right corner
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

/* Default values */
#define UNIVERSE_SIZE_DEFAULT 500
#define NUM_BIRDS_DEFUALT 128
#define NUM_ITERATIONS_DEFAULT 50
#define NUM_THREADS_DEFAULT 1
#define PRINTING 1 /* 1 enables printing, 0 disables printing */

/* Constants */
#define DEG_TO_RAD (M_PI / 180)
#define BIRD_SIZE 7
#define NEIGHBOR_RADIUS 20

/* Struct/Enum Definitions */
typedef struct {
  pthread_mutex_t count_lock;
  pthread_cond_t ok_to_proceed;
  int count;
} my_pthread_barrier_t;
typedef struct Bird {
  int id;
  int x, y;
  int dir;
  int next_x, next_y;
  int next_dir;
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

/* Function Declarations */
void * run_simulation( void *start_bird_p );
void decide_next_move( Bird *birds, int bird_index, Bird *b );
void apply_next_move( Bird *b );
void print( FILE * fout, Bird *birds, int sim_time, int csv_format );
int read_cl_args( int * argc_p, char *** argv_p );
void print_help_msg( void );
double distance (Bird *b1, Bird* b2 );
void my_pthread_init_barrier(my_pthread_barrier_t *b);
void my_pthread_barrier (my_pthread_barrier_t *b, int num_threads);


#endif
