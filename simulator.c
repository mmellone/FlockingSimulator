/* Flocking Simulator
 * Parallel Programming Final Project
 *
 * This project is a parallel discrete event simulation of flocking birds using
 * MPI and pThreads and is designed to run on the RPI BG/Q super computer AMOS
 *
 * Written by: Kevin O'Neill and Mitchell Mellone
 * Due: 2 May 2017
*/
#include "simulator.h"

int main(int argc, char **argv) {
  /* Declare local variables */
  int sim_time;
  //int start_index, end_index; /* Start/End of *birds that this rank is responsible for */
  Bird *birds; /* Array to hold ALL birds in simulation */
  int i, x, y, d;
  FILE * output_file;

  /* Start MPI */
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &commsize);
  MPI_Comm_rank(MPI_COMM_WORLD, &commrank);

  /* Initialize all parameters to defaults */
  universe_size = UNIVERSE_SIZE_DEFAULT;
  num_birds = NUM_BIRDS_DEFUALT;
  max_time = NUM_ITERATIONS_DEFAULT;
  num_threads = NUM_THREADS_DEFAULT;

  /* Initialize random number generator */
  srand((unsigned) time(NULL) * commrank);

  /* Read in arguments, update params if needed */
  if (read_cl_args(&argc, &argv) == EXIT_FAILURE) {
    return EXIT_FAILURE;
  }

  /* Define an MPI_Datatype for the Bird struct */
  MPI_Type_contiguous(BIRD_SIZE, MPI_INT, &MPI_Bird);
  MPI_Type_commit(&MPI_Bird);

  /* Create birds */
  birds_per_rank = num_birds / commsize;
  birds = malloc(birds_per_rank * sizeof(Bird));
  int start_id = birds_per_rank * commrank;
  /*
  * TODO import birds from input file rather than just randomly generate
  */
  for (i = 0; i < birds_per_rank; i++) {
    x = rand() % universe_size;
    y = rand() % universe_size;
    d = rand() % 360;
    birds[i] = (Bird) {
      .id = start_id + i,
      .x = x, .y = y, .dir = d,
      .next_x = x, .next_y = y, .next_dir = d
    };
  }

  /* Allocate an array to hold birds from all ranks */
  Bird* all_birds = malloc(num_birds*sizeof(Bird));
  if (all_birds == NULL) {
    perror("call to malloc() failed");
    MPI_Finalize();
    exit(EXIT_FAILURE);
  }

  output_file = fopen("simout.csv", "w");
  fprintf(output_file, "%s", "sim_time, x, y, direction\n");

  /* Run the simulation */
  for (sim_time = 0; sim_time < max_time; sim_time++) {
    /* Print to stdout in csv format */
    print(output_file, birds, sim_time, 1);

    /* Gather all of the birds from all ranks */
    MPI_Allgather(birds, birds_per_rank, MPI_Bird,
		  all_birds, birds_per_rank, MPI_Bird,
		  MPI_COMM_WORLD);


    for (i = 0; i < birds_per_rank; i++) {
      /* Calculate next moves */
      decide_next_move(all_birds, start_id + i, &birds[i]);
    }
    for (i = 0; i < birds_per_rank; i++) {
      /* Apply next moves after each move is calculated */
      apply_next_move(&birds[i]);
    }
  }


  free(all_birds);
  free(birds);
  MPI_Finalize();
}

/* Calculates the next move for bird b */
void decide_next_move( Bird *birds, int bird_index, Bird * b) {
  // Bird *b = &birds[bird_index];
  int i;
  int neighbor_count;
  double cohesion_x, cohesion_y, alignment_dir;

  neighbor_count = 0;
  cohesion_x = cohesion_y = alignment_dir = 0.0;

  /*
   * TODO implement separation
   */
  /* Calculate alignment, cohesion, and separation from neighbors */
  for (i = 0; i < num_birds; ++i) {
    if (i != bird_index && distance(b, &birds[i]) < NEIGHBOR_RADIUS) {
      ++neighbor_count;
      cohesion_x += birds[i].x;
      cohesion_y += birds[i].y;
      alignment_dir += birds[i].dir;
    }
  }

  double cohesion_dir = atan(cohesion_x / cohesion_y) * DEG_TO_RAD;
  alignment_dir = (int)(alignment_dir / neighbor_count) % 360;

  /* Calculate the next direction as an avg of the 3 rules */
  /* TODO- include separation */
  b->next_dir = (cohesion_dir + alignment_dir) / 2;


  /* Use trig values to calculate next position
     (N = 0, E = 90, S = 180, W = 270)     */
  double dx = cos(b->next_dir * DEG_TO_RAD)*10,
    dy = sin(b->next_dir * DEG_TO_RAD)*10;
  //int dx_int = dx > 0.5 ? 1 : dx < -0.5 ? -1 : 0,
  //    dy_int = dy > 0.5 ? 1 : dy < -0.5 ? -1 : 0;
  b->next_x = (int)(b->x + dx) % universe_size;
  b->next_y = (int)(b->y + dy) % universe_size;

  while (b->next_x < 0)
    b->next_x += universe_size;
  while (b->next_y < 0)
    b->next_y += universe_size;
}

/* Calculate the distance between two Birds */
double distance (Bird *b1, Bird* b2 ) {
  return sqrt(pow(b2->x - b1->x, 2.0) + pow(b2->y - b1->y, 2.0));
}

/* Applys the next move to a bird b */
void apply_next_move( Bird *b ) {
  b->x = b->next_x;
  b->y = b->next_y;
  b->dir = b->next_dir;
}

/*
 * Prints the current state of the simulation (aka full birds vector) to the
 * given file.
 * This should be called from all ranks, AFTER all information is updated
 *
 * csv_format should be set to 1 for writing data files and 0 for logging
 */
void print(FILE * fout, Bird *birds, int sim_time, int csv_format) {
  Bird* all_birds = NULL;
  if (commrank == 0) {
    all_birds = malloc(num_birds * sizeof(Bird));
    if (all_birds == NULL) {
      perror("call to malloc() failed");
      MPI_Finalize();
      exit(EXIT_FAILURE);
    }
  }

  MPI_Gather(birds, birds_per_rank, MPI_Bird,
	     all_birds, birds_per_rank, MPI_Bird,
	     0, MPI_COMM_WORLD);

  int i;
  if (commrank == 0) {
    if (!csv_format)
      fprintf(fout, "sim_time: %d\n", sim_time);
    Bird *b;
    for (i = 0; i < num_birds; i++) {
      b = &all_birds[i];

      if (csv_format)
	fprintf(fout, "%d, %d, %d, %d\n", sim_time, b->x, b->y, b->dir);
      else
	fprintf(fout, "  Bird %d: pos=(%d, %d), dir=%d\n",
		b->id, b->x, b->y, b->dir);
    }

    free(all_birds);
  }
}

/* Reads command line arguments according to help message (below) */
int read_cl_args( int * argc_p, char *** argv_p ) {
  int i, argc;
  char ** argv;
  argc = *argc_p;
  argv = *argv_p;
  for (i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-h") == 0) {
      /* Print help message and exit */
      print_help_msg();
      return EXIT_FAILURE;
    } else if (strcmp(argv[i], "-s") == 0) {
      if (i+1 >= argc) {
        fprintf(stderr, "%s", "ERROR: Expected int argument after -s\n");
        return EXIT_FAILURE;
      } else {
        i++;
        universe_size = atoi(argv[i]);
      }
    } else if (strcmp(argv[i], "-b") == 0) {
      if (i+1 >= argc) {
        fprintf(stderr, "%s", "ERROR: Expected int argument after -b\n");
        return EXIT_FAILURE;
      } else {
        i++;
        num_birds = atoi(argv[i]);
      }
    } else if (strcmp(argv[i], "-p") == 0) {
      if (i+1 >= argc) {
        fprintf(stderr, "%s", "ERROR: Expected int argument after -p\n");
        return EXIT_FAILURE;
      } else {
        i++;
        num_threads = atoi(argv[i]);
      }
    } else if (strcmp(argv[i], "-t") == 0) {
      if (i+1 >= argc) {
        fprintf(stderr, "%s", "ERROR: Expected int argument after -t\n");
        return EXIT_FAILURE;
      } else {
        i++;
        max_time = atoi(argv[i]);
      }
    } else {
      fprintf(stderr, "ERROR: Unrecognized argument flag %s\n", argv[i]);
      print_help_msg();
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}

/* Prints a help message to stdout */
void print_help_msg( void ) {
  printf("%s\n  %s\n    %s%d\n    %s%d\n    %s%d\n    %s%d\n",
         "Usage: ./a.out -s <size of universe> -b <number of birds> -t <number of sim iterations> -p <number of threads>",
           "Default Values:",
             "universe size - ", UNIVERSE_SIZE_DEFAULT,
             "number of birds - ", NUM_BIRDS_DEFUALT,
             "number of sim iterations - ", NUM_ITERATIONS_DEFAULT,
             "number of threads per MPI rank - ", NUM_THREADS_DEFAULT);
}
