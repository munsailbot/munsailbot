setwd("/Users/bryan/Git/munsailbot")
data <- read.csv("track.csv", sep=",", header = TRUE)

X1 <- min(data["Lon"])
X2 <- max(data["Lon"])
Y1 <- min(data["Lat"])
Y2 <- max(data["Lat"])

divx <- abs(X1-X2)/100
divy <- abs(Y1-Y2)/100
