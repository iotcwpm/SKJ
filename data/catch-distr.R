ll <- read.table('source/IOTC-2015-WPTT17-DATA08-_CEALL/IOTC-2015-WPTT17-DATA05- CELongline.csv',header=T,sep=',')
su <- read.table('source/IOTC-2015-WPTT17-DATA08-_CEALL/IOTC-2015-WPTT17-DATA06- CESurface.csv',header=T,sep=',')
co <- read.table('source/IOTC-2015-WPTT17-DATA08-_CEALL/IOTC-2015-WPTT17-DATA07- CECoastal.csv',header=T,sep=',')

ll$Catch <- ll$SKJ.MT
su$Catch <- rowSums(su[,c('SKJ.FS','SKJ.LS','SKJ.UNCL')],na.rm=T)
co$Catch <- co$SKJ.MT

cols <- c('Fleet','Gear','Year','MonthStart','MonthEnd','Grid','Catch')

# Bind together
ce <- rbind(
  ll[,cols],
  su[,cols],
  co[,cols]
)

# Only include records with a recorded catch
ce <- ce[!is.na(ce$Catch),]

# Only three records have MonthStart!=MonthEnd (they are annual data in 1991):
subset(ce,MonthStart!=MonthEnd)
# These are for Indonesian exploratory longline fishing and only represent <0.01% of catch 
sum(subset(ce,MonthStart!=MonthEnd)$Catch)/sum(ce$Catch)*100
# So just drop them, get rid of MonthEnd and rename MonthStart to Month
ce = subset(ce,MonthStart==MonthEnd)
ce = ce[,names(ce)!='MonthEnd']
names(ce)[names(ce)=='MonthStart'] = 'Month'

# Normalise gear
ce$Gear <- as.character(ce$Gear)
ce$Gear[ce$Gear %in% c('PSS')] <- 'PS'
ce$Gear[ce$Gear %in% c('BB','BBM','BBN')] <- 'PL'
ce$Gear[ce$Gear %in% c('GIL')] <- 'GN'
ce$Gear[!(ce$Gear %in% c('PS','PL','GN'))] <- 'OT'

# Decompose Grid
ce = within(ce,{
  GridSize = as.integer(substr(as.character(Grid),1,1))
  GridQuadrant = as.integer(substr(as.character(Grid),2,2))
  GridLat = as.integer(substr(as.character(Grid),3,4))
  GridLon = as.integer(substr(as.character(Grid),5,7))
  
  Lat = ifelse(GridQuadrant %in% c(1,4),1,-1) * (GridLat + c(2.5,5,5,10,0.5,2.5)[GridSize]) 
  Lon = ifelse(GridQuadrant %in% c(1,2),0,180) + (GridLon + c(5,10,5,10,0.5,2.5)[GridSize])
})

table(ce$GridSize)

# Cells of 10x20 (GridSize==2)
unique(subset(ce,GridSize==2,select=c('GridLat','GridLon')))

# Cells of 5x5 (GridSize==6)
unique(subset(ce,GridSize==6,select=c('GridLat','GridLon')))

# Cells of 1x1 (GridSize==5)
temp = ddply(
  # Only use 1x1 Grid and last ten years
  subset(ce,GridSize==5 & Year %in% 2005:2014),
  .(Gear,Lat,Lon),
  # Calculate an average catch
  summarise,Catch=sum(Catch)/10
)
areas = read.table('area-polygons.txt',header=T)
ggplot() + 
  geom_polygon(data=areas,aes(x=Lon,y=Lat,group=Area),colour='red',fill=NA) + 
  geom_polygon(data=map_data('world'),aes(x=long,y=lat,group=group),colour='grey70') + 
  geom_point(data=temp,aes(x=Lon,y=Lat,size=Catch,color=Gear),alpha=0.3) + 
  coord_map(xlim=c(20,160),ylim=c(-50,40)) + 
  scale_size_area(max_size=12)



# Save for use in documents
write.table(temp,file='input/catch_distr.tsv',col.names=T,row.names=F,sep='\t',quote=F)
