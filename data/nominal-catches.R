#!/usr/bin/Rscript

library(plyr)
library(reshape)
library(ggplot2)

#################################################
# Catch data by lat/lon

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


# Calculate the proportion of PS W catch that is in NW or SW by
# quart to apply below
temp <- within(subset(ce,Lon<75 & Gear=='PS'),{
  Quarter <- as.integer(Month/4)+1
  Region <- ifelse(Lat>-10,'NW','SW')
})
temp <- ddply(temp,.(Year,Quarter,Region),summarise,Catch=sum(Catch))
temp <- merge(temp,ddply(temp,.(Year,Quarter),summarise,Total=sum(Catch)))
temp <- within(temp,{
  Prop <- Catch/Total
})

ggplot(temp,aes(x=Year,y=Prop,colour=factor(Region))) + 
  geom_line() + 
  facet_wrap(~Quarter)

ps_w_props <- cast(temp,Year+Quarter~Region,value='Prop')
ps_w_props[is.na(ps_w_props)] <- 0
write.table(ps_w_props,file='input/catch-ps-sw-nw-props.tsv',row.names=F,quote=F,sep='\t')

#################################################3
# Catch data by quarter

# Read in data
ncq = read.table('source/CAS_Tables_SKJ_2013.txt',header=T,sep='\t',as.is=T)
ncq$tmt = as.numeric(ncq$tmt)

# Assign to operating model regions
ncq = within(ncq,{
  # Assign to an region based on IOTC_Area and Fleet
  # use unique(nc[,c('Fleet','EName')]) to get a Fleet name
  Region = NA
  Region[IOTC_Area=='IO_Eastern'] = 'EA'
  Region[IOTC_Area=='IO_Western' & Fleet=='MDV'] = 'MA'
  Region[IOTC_Area=='IO_Western' & Fleet!='MDV'] = 'NW' #Later we split PS catch into NW and SW by quarter
  Region = factor(Region,levels=c(
    'SW','NW','MA','EA'
  ))
})

# Summary to check above method coding
cast(ddply(ncq,.(Region,Gear),summarise,catch=sum(tmt)),Gear~Region)
ggplot(ddply(ncq,.(Region,Gear,Year),summarise,catch=log10(sum(tmt)))) + 
  geom_line(aes(x=Year,y=catch,colour=Gear,shape=Gear)) + 
  geom_point(aes(x=Year,y=catch,colour=Gear,shape=Gear),size=3) + 
  scale_shape_manual(values=1:27) +
  facet_wrap(~Region)

# Assign to operating model methods
ncq = within(ncq,{
  # Based on GearGroup defined in GearCode sheet of NCTROP.xlsx
  Method = NA
  Method[Gear %in% c('PS','PSS','RIN')] = 'PS'
  Method[Gear %in% c('BB','BBM','BBN')] = 'PL'
  Method[Gear %in% c('GILL','GIOF','GL')] = 'GN'
  Method[is.na(Method)] = 'OT'
  Method = factor(Method,levels=c(
    'PS','PL','GN','OT'
  ))
})

# Aggregate by OM areas, methods, years and quarters
sums = ddply(ncq,.(Region,Method,Year,Quarter),summarise,Catch=sum(tmt,na.rm=T))

# Split PS.NW into SW/NW
ps_w = subset(sums,Method=='PS' & Region=='NW')
ps_w = merge(ps_w,ps_w_props,by=c('Year','Quarter'))
ps_w = within(ps_w,{
  NW = Catch * NW
  SW = Catch * SW
})
ps_w = melt(ps_w[,c('Year','Quarter','Method','NW','SW')],id.vars=c('Year','Quarter','Method'))
names(ps_w)[names(ps_w)=='variable'] <- 'Region'
names(ps_w)[names(ps_w)=='value'] <- 'Catch'
  
sums <- rbind(
  ps_w,
  subset(sums,!(Method=='PS' & Region=='NW'))
)
sums$Region <- factor(sums$Region,levels=c('SW','NW','MA','EA'),ordered=T)

# Plot

ggplot(subset(sums,Method=='PS'),aes(x=Year,y=Catch,shape=factor(Quarter),color=factor(Quarter))) + 
  geom_line(alpha=0.5) + 
  geom_point(size=3,alpha=0.5) +
  facet_wrap(~Region) + 
  labs(y='Catch (t)')

svg("figs/nominal-catches-quarter.svg",width=25/2.54,height=18/2.54)
ggplot(sums) + 
  geom_line(aes(x=Year,y=Catch,color=Method),alpha=0.5) + 
  geom_point(aes(x=Year,y=Catch,color=Method,shape=Method),size=3,alpha=0.5) +
  facet_wrap(~Region) + 
  labs(y='Catch (t)')
dev.off()

svg("figs/nominal-catches-quarter-annual.svg",width=25/2.54,height=18/2.54)
ggplot(ddply(ncq,.(Region,Method,Year),summarise,Catch=sum(tmt,na.rm=T))) + 
  geom_line(aes(x=Year,y=Catch,color=Method),alpha=0.5) + 
  geom_point(aes(x=Year,y=Catch,color=Method,shape=Method),size=3,alpha=0.5) +
  facet_wrap(~Region) + 
  labs(y='Catch (t)')
dev.off()

# Rename columns, reorder, sort, recode and output for reading into model
names(sums) = c('year','quarter','method','region','catch')
sums = sums[,c('year','quarter','region','method','catch')]
sums = sums[with(sums,order(year,quarter,region,method)),]
sums = within(sums,{
  year = as.integer(as.character(year))
  quarter = quarter-1
  region = as.integer(region)-1
  method = as.integer(method)-1
  catch = round(catch)
})
write.table(sums,file='input/catches.tsv',row.names=F,quote=F,sep='\t')

ggplot(sums) + 
  geom_line(aes(x=year,y=catch,color=factor(quarter)),alpha=0.5) + 
  geom_point(aes(x=year,y=catch,color=factor(quarter),shape=factor(quarter)),size=3,alpha=0.5) +
  facet_grid(region~method) + 
  labs(y='Catch (t)')

# Calculate catch proportions by region/method for last 10 years
temp <- ddply(subset(sums,year>max(sums$year)-10),.(region,method),summarise,catch=sum(catch))
temp$value <- with(temp,round(catch/sum(catch),3))
temp <- merge(expand.grid(region=0:3,method=0:3),temp,all.x=T)
temp$catch <- NULL
temp$value[is.na(temp$value)]<- 0
write.table(temp,file='input/catch-prop-region-method.tsv',row.names=F,quote=F,sep='\t')

# Calculate catch proportions by region/method/quarter for last 10 years
temp <- ddply(subset(sums,year>max(sums$year)-10),.(region,method,quarter),summarise,catch=sum(catch))
temp$value <- with(temp,round(catch/sum(catch),3))
temp <- merge(expand.grid(region=0:3,method=0:3,quarter=0:3),temp,all.x=T)
temp$catch <- NULL
temp$value[is.na(temp$value)]<- 0
write.table(temp,file='input/catch-prop-region-method-quarter.tsv',row.names=F,quote=F,sep='\t')


#################################################3
# Catch data by year

# The sheet "Catch_Captures" from "IOTC-2015-WPTT17-DATA04-NC.xlsx" 
# was copied and pasted into file "source/IOTC-2015-WPTT17-DATA04-NC.txt"

# Read in data
nc <- read.table('source/IOTC-2015-WPTT17-DATA04-NC.txt',header=T,sep='\t')

# Assign to operatng model areas and methods and quarters
nc <- within(nc,{
    # Assign to an area based on area and fleet
    AreaOM = NA
    AreaOM[AreaIOTC=='Eastern Indian Ocean'] = 'EA'
    AreaOM[AreaIOTC=='Western Indian Ocean' & FlCde=='MDV'] = 'MA'
    AreaOM[AreaIOTC=='Western Indian Ocean' & FlCde!='MDV'] = 'NW'
    AreaOM =factor(AreaOM,levels=c(
        'NW','MA','EA'
    ))
    
    # Assign to a method
    Method = NA
    Method[GrGroup=='Purse Seine'] = 'PS'
    Method[GrGroup=='Baitboat'] = 'PL'
    Method[GrGroup=='Gillnet'] = 'GN'
    Method[GrGroup=='Other' | GrGroup=='Line' | GrGroup=='Longline'] = 'OT'
    Method =factor(Method,levels=c(
        'PS','PL','GN','OT'
    ))

    # Assign to a quarter
    Quarter = 1
})

# Get average annual catches by method for the top fleets other than EU and Maldives
# for which we do not have spatial information on catches
temp <- ddply(
  subset(nc,SpCde=='SKJ' & (Year.An %in% 2005:2014) & (FlCde %in% c('IDN','LKA','IRN','IND','PAK'))),
  .(FlCde,AreaOM,Method),
  summarise,Catch=sum(Catch.Capture.t.,na.rm=T)/10
)
write.table(temp,file="input/catch_distr_other.tsv",col.names=T,row.names=F,quote=F,sep='\t')

# Aggregate by OM areas, methods, years and quarters
sums = ddply(nc,.(AreaOM,Method,Year,Quarter),summarise,Catch=sum(SKJ,na.rm=T))

# Plot
svg("nominal-catches.svg",width=25/2.54,height=18/2.54)
ggplot(sums) + 
    geom_line(aes(x=Year,y=Catch,color=Method),alpha=0.5) + 
    geom_point(aes(x=Year,y=Catch,color=Method,shape=Method),size=3,alpha=0.5) +
    facet_wrap(~AreaOM) + 
    labs(y='Catch (t)')
dev.off()

# Rename columns and output
names(sums) = c('area','method','year','quarter','catch')
write.table(sums,file='input/nominal-catches.tsv',row.names=F,quote=F,sep='\t')


