library(ggplot2)

sim.dat <- read.csv("simout.csv") # Load simulation csv output

arrow_len <- 10 # Length of arrow
arrow_point <- 0.1 # Size of arrow-head
sim_times_to_print <- c(0, 10, 20, 30, 40, 49) # List of simulation times to plot
# sim_times_to_print <- unique(sim.dat$sim_time) for everything # Uncomment to print everything

sim.dat$sim_time <- as.factor(sim.dat$sim_time)
sim.dat$dir_rad <- sim.dat$direction*pi/180
sim.dat$dx <- (arrow_len*cos(sim.dat$dir_rad))
sim.dat$dy <- (arrow_len*sin(sim.dat$dir_rad))

for (time in sim_times_to_print) {
  print(
    ggplot(data=subset(sim.dat, sim_time==time), aes(x=x, y=y)) +
      geom_segment(aes(xend=x+dx, yend=y+dy), arrow=arrow(length = unit(arrow_point,"cm"))) +
      ggtitle( paste0("World during Simulation Time: ", time) )
  )
}