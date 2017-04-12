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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <time.h>
#include <math.h>

/* Constants */
#define UNIVERSE_SIZE_DEFAULT 500
#define NUM_BIRDS_DEFUALT 100
#define NUM_ITERATIONS_DEFAULT 1
#define NUM_THREADS_DEFAULT 1

#define DEG_TO_RAD M_PI / 180

/* Global Variables */
int universe_size; /* Width/height of the universe */
int num_birds; /* Number of birds to simulate */
int birds_per_rank; /* Number of birds simulated by each rank */
int max_time;
int num_threads; /* Number of threads per mpi process */
int commsize;  /* the number of MPI Ranks */
int commrank;  /* the MPI rank of this process */

MPI_Datatype MPI_Bird;  /* an MPI_Datatype for the Bird struct below */

/* Struct/Enum Definitions */
typedef enum Direction {
  N=0, NE=45, E=90, SE=135, S=180, SW=225, W=270, NW=315
} Direction;

typedef struct Bird {
  int id;
  int x, y;
  int dir;
  int next_x, next_y;
  int next_dir;
} Bird;

/* Function Declarations */
void decide_next_move( Bird *b );
void apply_next_move( Bird *b );
void print( FILE * fout, Bird *birds, int sim_time, int csv_format );
int read_cl_args( int * argc_p, char *** argv_p );
void print_help_msg( void );

#endif