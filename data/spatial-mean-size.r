require(reshape)
require(plyr)
require(ggplot2)

source('../common.R')

# Read in data
sf = read.table('source/FL_SKJ.txt',header=T)

# Decompose Grid
sf = within(sf,{
  GridSize = as.integer(substr(as.character(Grid),1,1))
  GridQuadrant = as.integer(substr(as.character(Grid),2,2))
  GridLat = as.integer(substr(as.character(Grid),3,4))
  GridLon = as.integer(substr(as.character(Grid),5,7))
  
  Lat = ifelse(GridQuadrant %in% c(1,4),1,-1) * (GridLat + c(2.5,5,5,10,0.5,2.5)[GridSize]) 
  Lon = ifelse(GridQuadrant %in% c(1,2),0,180) + (GridLon + c(5,10,5,10,0.5,2.5)[GridSize])
})
table(sf$GridSize)
sf = subset(sf,Gear=='PS' & GridSize==6)

# Names of columns with counts by size
cols <- sprintf('C%03d',1:150)

# Aggregate by lat/lon
agged <- ddply(sf,.(Lat,Lon),function(sub){
  colSums(sub[,cols])
})

# Do matrix multiplication
counts <- as.matrix(agged[,cols])
sizes <- 10:159
prods <- counts %*% sizes
sums <- rowSums(counts)
means <- prods[,1]/sums

# Link back to lat/lons etc
agged$sum <- sums
agged$mean <- means
# Some outliers - remove for better visualisation of scales
agged <- subset(agged,mean>=46 & mean<=52)

# Plot it up
land <- subset(map_data("world"),long>30 & long<130 & lat<30 & lat>-40)

ggplot(NULL) + 
  geom_point(data=agged,aes(x=Lon+2.5,y=Lat+2.5,colour=mean,size=sum),alpha=0.7) + 
  geom_polygon(data=land,aes(x=long, y=lat, group=group),fill='white',colour='grey60') +
  scale_size_area(max_size=30) + 
  coord_map('mercator',xlim=c(35,122),ylim=c(-32,27)) + 
  labs(colour='Mean size (cm)',size='N',x='',y='') + 
  scale_colour_gradientn(colours = rainbow(30))

ggplot(NULL) + 
  geom_tile(data=agged,aes(x=Lon,y=Lat,fill=mean,alpha=log(sum))) + 
  geom_polygon(data=land,aes(x=long, y=lat, group=group),fill='white',colour='grey60')
