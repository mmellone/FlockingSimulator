library(ggplot2)
library(rgl)

sim.dat <- read.csv("simout.csv") # Load simulation csv output
t.dat <- subset(sim.dat, sim_time==10)
plot3d(t.dat$x, t.dat$y, t.dat$z)
Sys.sleep(100)