library(ggplot2)

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
sim.dat$dx <- (arrow_len*cos(sim.dat$direction))
sim.dat$dy <- (arrow_len*sin(sim.dat$direction))

for (time in sim_times_to_print) {
  print(
    ggplot(data=subset(sim.dat, sim_time==time), aes(x=x-dx/2, y=y-dy/2)) +
      geom_segment(aes(xend=x+dx/2, yend=y+dy/2), arrow=arrow(length = unit(arrow_point,"cm"))) +
      scale_x_continuous(limits=world_limits, expand=c(0, 0)) +
      scale_y_continuous(limits=world_limits, expand=c(0,0)) +
      ggtitle( paste0("World during Simulation Time: ", time) )
  )
}
