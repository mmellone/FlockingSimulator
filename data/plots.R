library(ggplot2)
library(scales)
library(reshape)

# Results of the strong scaling study using random initial bird placements
ss <- read.csv('strongscaling2.csv')
ss$nodes <- (ss$ranks * ss$threads)
ss$threads <- factor(ss$threads)


# Total execution time between number of nodes
ggplot(data=ss, aes(x=nodes)) +
  geom_line(aes(y=total, color=threads)) +
  geom_point(aes(y=total, color=threads)) +
  scale_x_continuous(trans=log2_trans()) +
  scale_y_log10() +
  theme(legend.position = c(0.8,0.7),
        legend.background = element_rect(fill = NA),
        panel.background = element_rect(fill = 'white', colour = 'black'),
        panel.grid.major = element_line(colour = 'gray'),
        panel.grid.minor = element_line(colour = 'light gray'),
        text=element_text(size=18)) +
  ggtitle("Strong Scaling Study Execution Time") +
  labs(x="Ranks * Threads (log scale)", y="Time (in seconds, log scale)", color="Threads per Rank")
ggsave("plots/total_time.pdf")


# Total speedup
total.base <- ss[ss$nodes==64 & ss$threads==1,]$total  # Change to ss$nodes==1
ss$total.speedup <- total.base / ss$total
ggplot(data=ss, aes(x=nodes)) +
  geom_line(aes(y=total.speedup, color=threads)) +
  geom_point(aes(y=total.speedup, color=threads)) +
  theme(legend.position = c(0.22,0.7),
        legend.background = element_rect(fill = NA),
        panel.background = element_rect(fill = 'white', colour = 'black'),
        panel.grid.major = element_line(colour = 'gray'),
        panel.grid.minor = element_line(colour = 'light gray'),
        text=element_text(size=18)) +
  ggtitle("Total Speedup") +
  labs(x="Ranks * Threads", y="Speedup over 1 Node, 1 pthread", color="Threads per Rank")
ggsave("plots/total_speedup.pdf")


# Total speedup within same number nodes (thread speedup)
node.base <- list()
for (n in unique(ss$nodes)) {
  node.base[[n]] <- ss[ss$nodes==n & ss$threads==1,]$total
}
ss$thread.total.speedup <- unlist(node.base[ss$nodes]) / ss$total
ggplot(data=ss, aes(x=nodes)) +
  geom_line(aes(y=thread.total.speedup, color=threads)) +
  geom_point(aes(y=thread.total.speedup, color=threads)) +
  theme(legend.position = c(0.22,0.7),
        legend.background = element_rect(fill = NA),
        panel.background = element_rect(fill = 'white', colour = 'black'),
        panel.grid.major = element_line(colour = 'gray'),
        panel.grid.minor = element_line(colour = 'light gray'),
        text=element_text(size=18)) +
  ggtitle("Thread Speedup") +
  labs(x="Ranks * Threads", y="Speedup over Single pthread", color="Threads per Rank")
ggsave("plots/thread_speedup.pdf")


# Compute execution time
ggplot(data=ss, aes(x=nodes)) +
  geom_line(aes(y=compute, color=threads)) +
  geom_point(aes(y=compute, color=threads)) +
  theme(legend.position = c(0.8,0.7),
        legend.background = element_rect(fill = NA),
        panel.background = element_rect(fill = 'white', colour = 'black'),
        panel.grid.major = element_line(colour = 'gray'),
        panel.grid.minor = element_line(colour = 'light gray'),
        text=element_text(size=18)) +
  ggtitle("Compute Time") +
  scale_x_continuous(trans=log2_trans()) +
  scale_y_log10() +
  labs(x="Ranks * Threads (log scale)", y="Time (in seconds, log scale)", color="Threads per Rank")
ggsave("plots/compute_time.pdf")


# Communication execution time
ggplot(data=ss, aes(x=nodes)) +
  geom_line(aes(y=communicate, color=threads)) +
  geom_point(aes(y=communicate, color=threads)) +
  ggtitle("Communication Time") +
  theme(legend.position = c(0.8,0.3),
        legend.background = element_rect(fill = NA),
        panel.background = element_rect(fill = 'white', colour = 'black'),
        panel.grid.major = element_line(colour = 'gray'),
        panel.grid.minor = element_line(colour = 'light gray'),
        text=element_text(size=18)) +
  scale_x_continuous(trans=log2_trans()) +
  scale_y_log10() +
  labs(x="Ranks * Threads (log scale)", y="Time (in seconds, log scale)", color="Threads per Rank")
ggsave("plots/comm_time.pdf")


# Percent communication
ss$percentComm <- (ss$communicate / ss$total) * 100.0
ggplot(data=ss, aes(x=nodes)) +
  geom_line(aes(y=percentComm, color=threads)) +
  geom_point(aes(y=percentComm, color=threads)) +
  ggtitle("Percent Time Spent on Communication") +
  theme(legend.position = c(0.3,0.7),
        legend.background = element_rect(fill = NA),
        panel.background = element_rect(fill = 'white', colour = 'black'),
        panel.grid.major = element_line(colour = 'gray'),
        panel.grid.minor = element_line(colour = 'light gray'),
        text=element_text(size=18)) +
  scale_x_continuous(trans=log2_trans()) +
  # scale_y_log10() +
  labs(x="Ranks * Threads (log scale)", y="Percent of Total Time", color="Threads per Rank")
ggsave("plots/comm_percent.pdf")


# Weak Scaling test
ws <- read.csv('weakscaling.csv')
ws$nodes <- (ws$ranks * ws$threads)
ws$threads <- factor(ws$threads)
ggplot(data=ws, aes(x=nodes, y=total)) +
  geom_line() + geom_point() +
  scale_x_continuous(trans=log2_trans()) +
  scale_y_log10() +
  theme(panel.background = element_rect(fill = 'white', colour = 'black'),
        panel.grid.major = element_line(colour = 'gray'),
        panel.grid.minor = element_line(colour = 'light gray'),
        text=element_text(size=18)) +
  ggtitle("Weak Scaling Study Execution Time") +
  labs(x="Ranks * Threads (log scale)", y="Time (in seconds, log scale)")
ggsave("plots/weakscaling.pdf")


# Events per second weak scaling
ws$events <- 16*ws$nodes*1000
ws$events.per.second <- ws$events / ws$total
ggplot(data=ws, aes(x=nodes, y=events.per.second)) +
  geom_line() + geom_point() +
  scale_x_continuous(trans=log2_trans()) +
  # scale_y_log10() +
  theme(panel.background = element_rect(fill = 'white', colour = 'black'),
        panel.grid.major = element_line(colour = 'gray'),
        panel.grid.minor = element_line(colour = 'light gray'),
        text=element_text(size=18)) +
  ggtitle("Events Per Second") +
  labs(x="Ranks * Threads (log scale)", y="Number of Events per Second")
ggsave("plots/eventspersec.pdf")
