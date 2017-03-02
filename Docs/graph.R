library(ggplot2)

setwd("/Users/bryan/Git/munsailbot/Docs")

# Import CSV and delete empty/NA rows
# TODO: Use aes to define evenly spaced X/Y ticks
data <- read.table('track.csv', header = TRUE, sep =',')
data <- subset(data, Lat!=c("99.99"))
data <- subset(data, Lon!=c("99.99"))
state <- c(data$Sailstate)
dist <- c(data$DistanceToPoint)

ss <- function(x){
  ifelse(x == 0, 1,
    ifelse(x == 1, 2,
      ifelse(x == 2, 3,
        ifelse(x == 3, 4,
          ifelse(x == 4, 5, 0)
))))}

dtp <- function(x){
  ifelse(x < 20, "blue", "red");
}

ggplot(data,aes(x=data$Lon, y=data$Lat)) +
  # Graphing reported points
  geom_point(color=dtp(dist), (aes(size = ss(state)))) +
  # Removing X/Y labels
  labs(x="",y="") +
  # Plotting path
  geom_path() +
  # X/Y graphs are equivalent
  coord_equal() +
  # Remove legend
  theme(legend.position = "none")

#fin
