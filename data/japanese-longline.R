# Script for analysing Japanese longline data as provided by A Fontaneu
# Currently preliminary 

require(ggplot2)

regions <- read.table('area-polygons.txt',header=T)
names(regions) <- c('region','lat','lon')

data <- read.table('source/Japanese-Longline-1965-72.tsv',header=T)
data <- within(data,{
	quadrant = as.integer(substr(as.character(cell),1,1))
	lat = as.integer(substr(as.character(cell),2,3))
	lon = as.integer(substr(as.character(cell),4,6))
	
	lat = ifelse(quadrant %in% c(1,4),1,-1) * (lat + 2.5) 
	lon = ifelse(quadrant %in% c(1,2),0,180) + (lon + 2.5)
})

ggplot(subset(map_data("world"),long>30 & long<130 & lat<30 & lat>-40)) +
	geom_polygon(data=regions,aes(x=lon,y=lat,fill=region),colour='grey30',alpha=0.5) +
	geom_polygon(aes(x=long, y=lat, group=group),fill='white',colour='grey60') +
	geom_point(data=data,aes(x=lon,y=lat,size=SKJ),alpha=0.4) +
	scale_y_continuous('',expand=c(0,0)) +
	scale_x_continuous('',expand=c(0,0)) + 
	scale_size_area(max_size=10) + 
	coord_map('mercator') + 
	labs(fill='Region')
