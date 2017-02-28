# Greater precision required for mapping
library(ggplot2)

setwd("/Users/bryan/Git/munsailbot")

# Import CSV and delete empty/NA rows
data <- read.table('track.csv', header = TRUE, sep =',')
data <- subset(data, Lat!=c("99.99",""),select = c(Lon,Lat))
data1 <- matrix(c(47.57531,47.57530,47.57533,47.57534,47.57535,-52.73621,-52.73622,-52.73623,-52.73624,-52.73623), ncol=2)
tab <- table(data$Lon,data$Lat)
#plot(data1,type="o",ylab="Lon",xlab="Lat")
ggplot(data,aes(x=data$Lon, y=data$Lat)) + geom_point(data=data, aes(x=data$Lon, y=data$Lat), color="red") + labs(x="",y="") + geom_path()

# Extrema
X1 <- min(data["Lon"])
X2 <- max(data["Lon"])
Y1 <- min(data["Lat"])
Y2 <- max(data["Lat"])

# Creating a 100x100 grid
divx <- abs(X1-X2)/100
divy <- abs(Y1-Y2)/100
