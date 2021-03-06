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
  int i, x, y, z;
  double dx, dy, dz;
  pthread_t * threads;
  
  /* Start MPI */
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &commsize);
  MPI_Comm_rank(MPI_COMM_WORLD, &commrank);
  
  /* Initialize barrier for pthreads */
  pthread_barrier = malloc(sizeof(my_pthread_barrier_t));
  my_pthread_init_barrier(pthread_barrier);

  /* Initialize all parameters to defaults */
  universe_size = UNIVERSE_SIZE_DEFAULT;
  num_birds = NUM_BIRDS_DEFUALT;
  max_time = NUM_ITERATIONS_DEFAULT;
  num_threads = NUM_THREADS_DEFAULT;
  import_from_file = IMPORT_FROM_FILE_DEFAULT;
  bird_speed = (double) universe_size / BIRD_SPEED_FACTOR;
  
  /* Initialize random number generator */
  srand((unsigned) time(NULL) * commrank);

  /* Read in arguments, update params if needed */
  if (read_cl_args(&argc, &argv) == EXIT_FAILURE) {
    return EXIT_FAILURE;
  }
  
  /* Define an MPI_Datatype for the Bird struct */
  MPI_Type_contiguous(BIRD_SIZE, MPI_FLOAT, &MPI_Bird);
  MPI_Type_commit(&MPI_Bird);

  /* Create birds */
  birds_per_rank = num_birds / commsize;
  birds_per_thread = birds_per_rank / num_threads;
  birds = malloc(birds_per_rank * sizeof(Bird));
  int start_id = birds_per_rank * commrank;

  if (import_from_file) {
    spawn_birds_file(start_id);
  } else {
    spawn_birds_randomly(start_id);
  }

  /* Allocate an array to hold birds from all ranks */
  all_birds = malloc(num_birds*sizeof(Bird));
  if (all_birds == NULL) {
    perror("call to malloc() failed");
    MPI_Finalize();
    exit(EXIT_FAILURE);
  }

  /* Set up printing */
  if (PRINTING) {
    output_file = fopen("simout.csv", "w");
    fprintf(output_file, "sim_time, x, y, z, dx, dy, dz\n");
  }

  /* Start the global timer */
#ifdef BGQ
  unsigned long long start_time;
#else
  double start_time;
#endif
  if (commrank == 0) {
#ifdef BGQ
    start_time = GetTimeBase();
#else
    start_time = MPI_Wtime();
#endif
  }

  /* Spawn threads if applicable and run simulation */
  threads = malloc((num_threads-1) * sizeof(pthread_t));
  for (i = 0; i < num_threads-1; i++) {
    int * start_bird = malloc(sizeof(int));
    *start_bird = i * birds_per_thread;
    pthread_create(&threads[i], NULL, &run_simulation, start_bird);
  }
  int * start_bird = malloc(sizeof(int));
  *start_bird = (num_threads-1) * birds_per_thread;
  run_simulation(start_bird);

  for (i = 0; i < num_threads-1; i++) {
    pthread_join(threads[i], NULL);
  }

  MPI_Barrier(MPI_COMM_WORLD);

  /* Write execution time to stdout */
  if (commrank == 0) {
#ifdef BGQ
    if (CSV_STATS) {
      printf("%d,%d,%f,%f,%f\n", commsize, num_threads,
             (double)(GetTimeBase() - start_time) / (double) CLOCK_RATE,
             (double) comp_time / (double) CLOCK_RATE,
             (double) comm_time / (double) CLOCK_RATE);
    } else {
      printf("Total time:             %f seconds\n",
       (double)(GetTimeBase() - start_time) / (double) CLOCK_RATE);
      printf("Computation time:       %f seconds\n",
       (double)comp_time / (double) CLOCK_RATE);
      printf("MPI Communication time: %f seconds\n",
       (double) comm_time / (double) CLOCK_RATE);
    }

#else
    if (CSV_STATS) {
      printf("%d,%d,%f,%f,%f\n", commsize, num_threads,
             MPI_Wtime() - start_time,
             comp_time,
             comm_time);
    } else {
      printf("Total time:             %f seconds\n", MPI_Wtime() - start_time);
      printf("Computation time:       %f seconds\n", comp_time);
      printf("MPI Communication time: %f seconds\n", comm_time);
    }
#endif
  }

  /* Clean up dynamic memory and close MPI */
  free(all_birds);
  free(birds);
  free(pthread_barrier);
  MPI_Finalize();
}


/**
 * Run the simulation with the given starting index
 */
void * run_simulation(void * start_bird_p) {
  int i, sim_time;
  int start_bird = *(int*)start_bird_p;
  int thread_id = start_bird / birds_per_thread;
  free(start_bird_p);

  /* Declare start time variables */
#ifdef BGQ
  unsigned long long start_time;
#else
  double start_time;
#endif

  /* Run the simulation */
  for (sim_time = 0; sim_time < max_time; sim_time++) {
    if (thread_id == 0) {
      /* Print to stdout in csv format */
      if (PRINTING)
	print(output_file, birds, sim_time, 1);

      /* Start the MPI timer for communication time */
      if (commrank == 0) {
#ifdef BGQ
	start_time = GetTimeBase();
#else
	start_time = MPI_Wtime();
#endif
      }

      /* Gather all of the birds from all ranks */
      MPI_Allgather(birds, birds_per_rank, MPI_Bird,
        all_birds, birds_per_rank, MPI_Bird,
        MPI_COMM_WORLD);

      /* Add the time spent in communication to comm_time */
      if (commrank == 0) {
#ifdef BGQ
	comm_time += GetTimeBase() - start_time;
#else
	comm_time += MPI_Wtime() - start_time;
#endif
      }
    }

    /* Start the MPI timer for computation time */
    if (commrank == 0 && thread_id == 0) {
#ifdef BGQ
      start_time = GetTimeBase();
#else
      start_time = MPI_Wtime();
#endif
    }

    /* Calculate next moves */
    my_pthread_barrier(pthread_barrier, num_threads);
    for (i = start_bird; i < start_bird + birds_per_thread; i++)
      decide_next_move(all_birds, start_bird + i, &birds[i]);
    
    /* Apply moves after all moves are calculated */
    my_pthread_barrier(pthread_barrier, num_threads);
    for (i = start_bird; i < start_bird + birds_per_thread; i++)
      apply_next_move(&birds[i]);

    /* Add time spent in computation to comp_time */
    my_pthread_barrier(pthread_barrier, num_threads);
    if (commrank == 0 && thread_id == 0) {
#ifdef BGQ
      comp_time += GetTimeBase() - start_time;
#else
      comp_time += MPI_Wtime() - start_time;
#endif
    }
  }
  return NULL;
}

/* Calculates the next move for bird b */
void decide_next_move(Bird *birds, int bird_index, Bird * b) {
  int i;
  int neighbor_count;
  double alignment_x, alignment_y, alignment_z,
    cohesion_x, cohesion_y, cohesion_z,
    separation_x, separation_y, separation_z;

  neighbor_count = 0;
  alignment_x = alignment_y = alignment_z =
    cohesion_x = cohesion_y = cohesion_z =
    separation_x = separation_y = separation_z = 0.0;

  /* Calculate alignment, cohesion, and separation from neighbors */
  for (i = 0; i < num_birds; ++i) {
    if (i != bird_index && distance(b, &birds[i]) < NEIGHBOR_RADIUS) {
      ++neighbor_count;

      /* Alignment is the average direction of neighbors */
      alignment_x += birds[i].dx;
      alignment_y += birds[i].dy;
      alignment_z += birds[i].dz;

      /* Cohesion is the average position of neighbors */
      cohesion_x += b->x + delta(b->x, birds[i].x);
      cohesion_y += b->y + delta(b->y, birds[i].y);
      cohesion_z += b->z + delta(b->z, birds[i].z);

      /* Separation is the negative distance to neighbors */
      double d = distance(b, &birds[i]);
      if (d > 0 && d <= SEPARATION_RADIUS) {
	double dx = delta(b->x, birds[i].x),
	  dy = delta(b->y, birds[i].y),
	  dz = delta(b->z, birds[i].z);
	normalize(&dx, &dy, &dz, 1.0);
	separation_x -= dx / d;
	separation_y -= dy / d;
	separation_z -= dz / d;
      }
    }
  }
  
  if (neighbor_count > 0) {
    /* divide out all averages by neighbor count */
    alignment_x /= neighbor_count;
    alignment_y /= neighbor_count;
    alignment_z /= neighbor_count;
    cohesion_x = (cohesion_x / neighbor_count) - b->x;
    cohesion_y = (cohesion_y / neighbor_count) - b->y;
    cohesion_z = (cohesion_z / neighbor_count) - b->z;
    separation_x /= neighbor_count;
    separation_y /= neighbor_count;
    separation_z /= neighbor_count;

    /* normalize each vector */
    normalize(&alignment_x, &alignment_y, &alignment_z, 1.0);
    normalize(&cohesion_x, &cohesion_y, &cohesion_z, 1.0);
    normalize(&separation_x, &separation_y, &separation_z, 1.0);

    /* Calculate the next direction as an avg of the 3 rules */
    double dx = alignment_x + cohesion_x + separation_x,
      dy = alignment_y + cohesion_y + separation_y,
      dz = alignment_z + cohesion_z + separation_z;
    normalize(&dx, &dy, &dz, bird_speed);
    b->next_dx = dx;
    b->next_dy = dy;
    b->next_dz = dz;
  } else {
    /* If no neighbors are found, continue in the same direction */
    b->next_dx = b->dx;
    b->next_dy = b->dy;
    b->next_dz = b->dz;
  }
}

/**
 *  Calculate the distance between two coordinates along
 *   one axis, accounting for wraparound
 */
double delta( double d1, double d2 ) {
  double delta = d2 - d1;
  if (delta > universe_size/2.0 || delta < universe_size/-2.0)
    delta = universe_size - delta;
  return delta;
}

/**
 * Calculate the distance between two Birds
 *  NOTE: this accounts for wraparound over edges
 */
double distance (Bird *b1, Bird* b2 ) {
  double dx = delta(b2->x, b1->x),
    dy = delta(b2->y, b1->y),
    dz = delta(b2->z, b1->z);

  return sqrt(dx*dx + dy*dy + dz*dz);
}

/* Applies the next move to a bird b */
void apply_next_move( Bird *b ) {
  /* Update the direction */
  b->dx = b->next_dx;
  b->dy = b->next_dy;
  b->dz = b->next_dz;

  /* update the new position */
  b->x = (int)(b->x + b->dx + universe_size) % universe_size;
  b->y = (int)(b->y + b->dy + universe_size) % universe_size;
  b->z = (int)(b->z + b->dz + universe_size) % universe_size;
}


/**
 *   Normalize a (x,y) vector to unit length,
 *    then multiply to a factor of len
 */
void normalize(double *x, double *y, double *z, double len ) {
  double length = sqrt((*x)*(*x) + (*y)*(*y) + (*z)*(*z));
  if (length != 0) {
    *x /= length;
    *y /= length;
    *z /= length;
  }

  /* Multiply vector by len */
  if (len != 1.0) {
    *x *= len;
    *y *= len;
    *z *= len;
  }
}

/*
 * Prints the current state of the simulation (aka full birds vector) to the
 * given file.
 * This should be called from all ranks, AFTER all information is updated
 *
 * csv_format should be set to 1 for writing data files and 0 for logging
 */
void print(FILE * fout, Bird *birds, int sim_time, int csv_format) {
  Bird* birds_to_print = NULL;
  if (commrank == 0) {
    birds_to_print = malloc(num_birds * sizeof(Bird));
    if (birds_to_print == NULL) {
      perror("call to malloc() failed");
      MPI_Finalize();
      exit(EXIT_FAILURE);
    }
  }

  MPI_Gather(birds, birds_per_rank, MPI_Bird,
	           birds_to_print, birds_per_rank, MPI_Bird,
	           0, MPI_COMM_WORLD);

  int i;
  if (commrank == 0) {
    if (!csv_format)
      fprintf(fout, "sim_time: %d\n", sim_time);

    Bird *b;
    for (i = 0; i < num_birds; i++) {
      b = &birds_to_print[i];

      if (csv_format)
	fprintf(fout, "%d, %f, %f, %f, %f, %f, %f\n",
		sim_time, b->x, b->y, b->z, b->dx, b->dy, b->dz);
      else
	fprintf(fout, "  Bird %d: pos=(%f, %f, %f), dir=(%f, %f, %f)\n",
		b->id, b->x, b->y, b->z, b->dx, b->dy, b->dz);
    }

    free(birds_to_print);
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
	bird_speed = (double) universe_size / BIRD_SPEED_FACTOR;
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
    } else if (strcmp(argv[i], "-f") == 0) {
      import_from_file = 1;
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
         "Usage: ./a.out -s <size of universe> -b <number of birds> -t <number of sim iterations> -p <number of threads> -f (indicates to initialize birds from input file)",
           "Default Values:",
             "universe size - ", UNIVERSE_SIZE_DEFAULT,
             "number of birds - ", NUM_BIRDS_DEFUALT,
             "number of sim iterations - ", NUM_ITERATIONS_DEFAULT,
             "number of threads per MPI rank - ", NUM_THREADS_DEFAULT);
}

/* Spawns birds according to a File "rank_<ranknum>.birds"
   The file MUST have one bird per line in the format:
   x,y,direction
   These files can be generated by calling generateBirds.py
*/
void spawn_birds_file(int start_id) {
  // Have the first rank parse the input file
  int i, n, x, y, z;
  double dx, dy, dz;
  char input_file_name[FILE_NAME_BUFFER_SIZE];
  FILE * fin;

  /* Init local vars to parse file */
  snprintf(input_file_name, FILE_NAME_BUFFER_SIZE, "rank_%d.birds", commrank);
  fin = fopen(input_file_name, "r");
  if (fin == NULL) {
    fprintf(stderr, "Can't open file %s\n", input_file_name);
    MPI_Finalize;
    exit(EXIT_FAILURE);
  }

  for (i = 0; i < birds_per_rank; i++) {
    n = fscanf(fin, "%d,%d,%d,%lf,%lf,%lf\n", &x, &y, &z, &dx, &dy, &dz);
    if (n < 3) {
      fprintf(stderr, "File Read Error: %s\n  iteration: %d\n", input_file_name, i);
      perror("  Error msg");
      MPI_Finalize;
      exit(EXIT_FAILURE);
    } else {
      normalize(&dx, &dy, &dz, bird_speed);
      init_bird(start_id, i, x, y, z, dx, dy, dz);
    }
  }

  MPI_Barrier(MPI_COMM_WORLD);
}

void spawn_birds_randomly(int start_id) {
  int i, x, y, z;
  double dx, dy, dz;

  for (i = 0; i < birds_per_rank; i++) {
    x = rand() % universe_size;
    y = rand() % universe_size;
    z = rand() % universe_size;
    dx = rand() % (int) bird_speed;
    dy = rand() % (int) bird_speed;
    dz = rand() % (int) bird_speed;
    normalize(&dx, &dy, &dz, bird_speed);
    init_bird(start_id, i, x, y, z, dx, dy, dz);
  }

  MPI_Barrier(MPI_COMM_WORLD);
}

void init_bird(int start_id, int index, int x, int y, int z, double dx, double dy, double dz) {
  birds[index] = (Bird) {
    .id = start_id + index,
    .x = x, .y = y, .z = z,
    .dx = dx, .dy = dy, .dz = dz,
    .next_dx = dx, .next_dy = dy, .next_dz = dz,
  };
}

/* Pthread barrier functions, taken from 3/21 lecture notes */
void my_pthread_init_barrier(my_pthread_barrier_t *b) {
  b->count = 0;
  pthread_mutex_init(&(b->count_lock), NULL);
  pthread_cond_init(&(b->ok_to_proceed), NULL);
}

void my_pthread_barrier (my_pthread_barrier_t *b, int num_threads) {
  pthread_mutex_lock(&(b -> count_lock));
  b->count++;
  if (b->count == num_threads) {
    b->count = 0;
    pthread_cond_broadcast(&(b->ok_to_proceed));
  }
  else
    while (0 != pthread_cond_wait(&(b->ok_to_proceed), &(b->count_lock)));
  pthread_mutex_unlock(&(b->count_lock));
}
