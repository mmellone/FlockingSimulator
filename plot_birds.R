require("scatterplot3d")
library(scatterplot3d)

args <- commandArgs(TRUE)
size <- as.integer(args[1])
step <- as.integer(args[2])

sim.dat <- read.csv("simout.csv") # Load simulation csv output

arrow_len <- 10 # Length of arrow
arrow_point <- 0.1 # Size of arrow-head
max_time <- max(sim.dat$sim_time)
sim_times_to_print <- seq(0, max_time, step) # List of simulation times to plot
world_limits <- c(0, size) # Change to match universe_size
# sim_times_to_print <- unique(sim.dat$sim_time) # Uncomment to print everything

sim.dat$sim_time <- as.factor(sim.dat$sim_time)

for (time in sim_times_to_print) {
  t.dat <- subset(sim.dat, sim_time==time)

  with (t.dat,
        print(
          scatterplot3d(x=x, y=y, z=z,
                        pch=16,
                        main=paste0("World during Simulation Time: ", time))))

}
