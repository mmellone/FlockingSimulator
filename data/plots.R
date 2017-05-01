library(ggplot2)

# Results of the strong scaling study using random initial bird placements
ss <- read.csv('strongscaling.csv')
ss$nodes <- (ss$ranks * ss$threads) / 64
ss$threads <- factor(ss$threads)


# Total execution time between number of nodes
ggplot(data=ss, aes(x=nodes)) +
  geom_line(aes(y=total, color=threads)) +
  geom_point(aes(y=total, color=threads)) +
  scale_x_log10() +
  scale_y_log10() +
  ggtitle("Strong Scaling Study Execution Time") +
  labs(x="BG/Q Nodes (log scale)", y="Total Time (in seconds, log scale)", color="Threads per Rank")
ggsave("plots/total_time.png")


# Total speedup
total.base <- ss[ss$nodes==2 & ss$threads==1,]$total  # Change to ss$nodes==1
ss$total.speedup <- total.base / ss$total
ggplot(data=ss, aes(x=nodes)) +
  geom_line(aes(y=total.speedup, color=threads)) +
  geom_point(aes(y=total.speedup, color=threads)) +
  ggtitle("Total Speedup") +
  labs(x="BG/Q Nodes", y="Speedup over 1 Node, 1 pthread", color="Threads per Rank")
ggsave("plots/total_speedup.png")


# Total speedup within same number nodes (thread speedup)
node.base <- list()
for (n in unique(ss$nodes)) {
  node.base[[n]] <- ss[ss$nodes==n & ss$threads==1,]$total
}
ss$thread.total.speedup <- unlist(node.base[ss$nodes]) / ss$total
ggplot(data=ss, aes(x=nodes)) +
  geom_line(aes(y=thread.total.speedup, color=threads)) +
  geom_point(aes(y=thread.total.speedup, color=threads)) +
  ggtitle("Speedup over 1 pthread") +
  labs(x="BG/Q Nodes", y="Speedup over 1 Node, No Pthreads", color="Threads per Rank")
ggsave("plots/thread_speedup.png")


# Compute execution time
ggplot(data=ss, aes(x=nodes)) +
  geom_line(aes(y=compute, color=threads)) +
  geom_point(aes(y=compute, color=threads)) +
  ggtitle("Compute Time") +
  scale_x_log10() +
  scale_y_log10() +
  labs(x="BG/Q Nodes (log scale)", y="Time Spent in Compute(in seconds, log scale)", color="Threads per Rank")
ggsave("plots/compute_time.png")


# Communication execution time
ggplot(data=ss, aes(x=nodes)) +
  geom_line(aes(y=communicate, color=threads)) +
  ggtitle("Communication Time") +
  scale_x_log10() +
  scale_y_log10() +
  labs(x="BG/Q Nodes (log scale)", y="Time Spent in MPI Message Passing (in seconds, log scale)", color="Threads per Rank")
ggsave("plots/comm_time.png")


# Weak Scaling test
ws <- read.csv('weakscaling.csv')
ws$nodes <- (ws$ranks * ws$threads) / 64
ws$threads <- factor(ws$threads)
ggplot(data=ws, aes(x=nodes, y=total)) +
  geom_line() + geom_point() +
  scale_x_log10() + scale_y_log10() +
  ggtitle("Weak Scaling Study Execution Time") +
  labs(x="BG/Q Nodes (log scale)", y="Total Time (in seconds, log scale)")

ggsave("plots/weakscaling.png")
  