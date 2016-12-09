# Greater precision required for mapping
setwd("/Users/bryan/Git/munsailbot")

# Import CSV and delete empty/NA rows
data <- read.table('track.csv', header = TRUE, sep =',')
data <- subset(data, Lat!=c("99.99",""),select = c(Lon,Lat))
tab <- table(data$Lon,data$Lat)
plot(data,type="o")

# Extrema
X1 <- min(data["Lon"])
X2 <- max(data["Lon"])
Y1 <- min(data["Lat"])
Y2 <- max(data["Lat"])

# Creating a 100x100 grid
divx <- abs(X1-X2)/100
divy <- abs(Y1-Y2)/100
