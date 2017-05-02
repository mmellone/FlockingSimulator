library(rgl)

args <- commandArgs(TRUE)
universe_size <- as.integer(args[1])
filename_prefix <- args[2]
time <- as.integer(args[3])-1
bird_size <- universe_size / 200.0 # Set the sphere radius as 1/200th the graph size

sim.dat <- read.csv("simout.csv") # Load simulation csv output


for (i in 0:time) {
    t.dat <- subset(sim.dat, sim_time==i)

    open3d(windowRect=c(0,0,1000,1000))
    axes3d(edges="bbox", labels=FALSE, tick=FALSE)
    plot3d(t.dat$x, t.dat$y, t.dat$z, aspect=TRUE, type="s", radius=bird_size,
    		    xlim=c(0, universe_size), ylim=c(0, universe_size), zlim=c(0, universe_size),
            xlab="", ylab="", zlab="", main=paste0("Time: ", i))
    
    filename <- sprintf("%s/%04d.png", filename_prefix, i)
    rgl.snapshot(filename, "png", top=TRUE)
    rgl.close()
}
