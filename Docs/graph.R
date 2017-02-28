library(ggplot2)

setwd("/Users/bryan/Git/munsailbot")

# Import CSV and delete empty/NA rows
# TODO: Use aes to define evenly spaced X/Y ticks
data <- read.table('track.csv', header = TRUE, sep =',')
data <- subset(data, Lat!=c("99.99"))
data <- subset(data, Lon!=c("99.99"))
state <- c(data$Sailstate)

ss <- function(x){ifelse(x > 0, 5, 2)}

ggplot(data,aes(x=data$Lon, y=data$Lat)) +
  geom_point(color="red", (aes(size = ss(state)))) +
  labs(x="",y="") +
  geom_path() +
  coord_equal()

#fin
