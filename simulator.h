/* Flocking Simulator
 * Parallel Programming Final Project
 *
 * This project is a parallel discrete event simulation of flocking birds using
 * MPI and pThreads and is designed to run on the RPI BG/Q super computer AMOS
 *
 * Written by: Kevin O'Neill and Mitchell Mellone
 * Due: 2 May 2017
*/

#ifndef _SIMULATOR_H_
#define _SIMULATOR_H_

/* Constants */
#define UNIVERSE_SIZE_DEFAULT 500
#define NUM_BIRDS_DEFUALT 100
#define NUM_THREADS_DEFAULT 1

/* Global Variables */
int universe_size; /* Width/height of the universe */
int num_birds; /* Number of birds to simulate */
int num_threads; /* Number of threads per mpi process */

/* Function Declarations */
void print_help_msg( void );

#endif
