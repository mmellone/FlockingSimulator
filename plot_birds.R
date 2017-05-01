library(rgl)

filename_prefix <- "tmp"
universe_size <- 500

sim.dat <- read.csv("simout.csv") # Load simulation csv output

t.dat <- subset(sim.dat, sim_time==50)

for (i in 0:99) {
    t.dat <- subset(sim.dat, sim_time==i)

    open3d(windowRect=c(0,0,1000,1000))
    axes3d(edges="bbox", labels=FALSE, tick=FALSE)
    plot3d(t.dat$x, t.dat$y, t.dat$z, aspect=TRUE, type="s", size=0.5,
    		    xlim=c(0, universe_size),
		    ylim=c(0, universe_size),
		    zlim=c(0, universe_size))
    
    filename <- paste(filename_prefix, formatC(i, digits = 1, flag = "0"), ".png", sep = "")
    rgl.snapshot(filename, "png", top=TRUE)
    rgl.close()
}