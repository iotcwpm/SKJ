#!/usr/bin/Rscript

# A R script to process size frequency data

library(plyr)
library(reshape)
library(ggplot2)

###################################################
# Load and normalise data

# Read in data
sf = read.table('source/FL_SKJ.txt',header=T)

#   Notes on fields from FL_SKJ.xlsx 
#
#   Fleet    IOTC Fleet code
#   Year	Year
#   MonthStart	From month
#   MonthEnd	To month
#   Grid	Area reported
#   Gear	Gear IOTC code
#   SchoolType	Type of school (FS: free-school; LS: associated school; UNCL: Unclassified)
#   Species	Species (SKJ: skipjack tuna)
#   MeasureType	Type of measurement (FL: tip of lower jaw-fork of the tail length; EFL: Anterior eye orbit-fork of the tail length) 
#   FirstClassLow	First length class (corresponding to class C001)
#   SizeInterval	Interval (cm) between consecutive fork length classes
#   TnoFish	Total number of fish reported
#   TkgFish	Total kilograms of fish (estimated using IOTC length-weight relationship)
#   avKg	Average weight (kg)
#   C001…C150	Number of fish estimated size class C001 to C150
#
#   NOTE: Fleet and gear codes are described in NC_TROP.xlsx	

# A small number of records have MonthStart!=MonthEnd
# These are mainly JPN, LL
unique(subset(sf,MonthStart!=MonthEnd,select=c('Fleet','Year','Gear')))
# These represent only about 0.2% of records...
sum(with(sf,MonthStart!=MonthEnd))/nrow(sf)*100
# So just drop them, get rid of MonthEnd and rename MonthStart to Month
sf = subset(sf,MonthStart==MonthEnd)
sf = sf[,names(sf)!='MonthEnd']
names(sf)[names(sf)=='MonthStart'] = 'Month'

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

# See if any of the other fields are needed

table(sf$Gear)

# Species is all SKJ, remove it
table(sf$Species)
sf$Species <- NULL

table(sf$SchoolType)

# MeasureType is all FL, remove it
table(sf$MeasureType)
sf$MeasureType <- NULL

# FirstClassLow is all 10, remove it
table(sf$FirstClassLow)
sf$FirstClassLow <- NULL

# SizeInterval is all 1, remove it
table(sf$SizeInterval)
sf$SizeInterval <- NULL

###############################################################
# Allocate records to operating model strata : Region, Method, Year, Quarter

# 99% of the records use a GridSize of 5 (1x1 degreee cells) or 6 (5x5 degree cells)
# Remove records with other GridSizes because they may straddle Lon 80 and thus
# add complication to assigning records to areas
sum(sf$GridSize>=5)/nrow(sf)
sf = subset(sf,GridSize>=5)

# Assign to an operating model region
sf = within(sf,{
    # Because a 5x5 grid will straddle the 77 Lon line that is used to separate
    # Sri Lanka and Maldives, put all LKA records in E
    Region = NA
    Region[Lon>=20 & Lon<80 & Lat>-10] = 'NW'
    Region[Lon>=20 & Lon<80 & Lat<=-10] = 'SW'
    Region[(Lon>=80 & Lon<150) | Fleet=='LKA'] =  'EA'
    Region[Fleet=='MDV'] = 'MA'
    Region = factor(Region,levels=c(
        'SW','NW','MA','EA'
    ))
})
# Drop small number of rows report outside of regions
sf = subset(sf,!is.na(Region))

# Summarise size frequencies by gear to better determine how to aggregate to methods
cs = sprintf('C%03d',10:70)
sizes_gear = ddply(sf,.(Gear),function(sub)colSums(sub[,cs]))
total = rowSums(sizes_gear[,cs])
totals = data.frame(Gear=sizes_gear$Gear,n=total)
sizes_gear[,cs] = sizes_gear[,cs]/total
sizes_gear = melt(sizes_gear)
sizes_gear = within(sizes_gear,{
  size = as.character(variable)
  size = as.integer(substr(size,2,nchar(size)))
  size = size + 10
})
sizes_gear = subset(sizes_gear,Gear %in% subset(totals,n>100)$Gear)
sizes_gear = within(sizes_gear,{
  Method = NA
  
  Method[Gear %in% c('PS','PSS','RIN')] = 'PS'
  
  Method[Gear=='BB'] = 'PL'
  Method[Gear=='UNCL'] = 'PL'
  
  Method[Gear %in% c('GILL','GIHA','G/L')] = 'GN'
  
  Method[is.na(Method)] = 'OT'
})
ggplot(sizes_gear,aes(x=size,y=value,colour=Gear)) + 
  geom_point() + geom_line() + 
  facet_wrap(~Method)

# Assign method and quarter
sf = within(sf,{
    # Method
    # Used LargeGroup on CodeGear sheet in SF_Reference.xlxs as a guide
    Method = NA
    Method[Gear %in% c('PS','PSS','RIN')] = 'PS'
    
    Method[Gear=='BB'] = 'PL'
    # During 1997,98,99 there is no BB samples for maldives, only UNCL samples
    # Assume that those are PL
    Method[Fleet=='MDV' & Gear=='UNCL'] = 'PL'
    
    Method[Gear %in% c('GILL','GIHA','G/L')] = 'GN'
 
    Method[is.na(Method)] = 'OT'
    
    Method = factor(Method,levels=c(
        'PS','PL','GN','OT'
    ))
    
    # Quarter
    Quarter = floor((Month-1)/3)+1
})
# Check no NAs or drop NAs
table(sf$Region,sf$Fleet)
table(sf$Method,sf$Fleet)
table(sf$Gear,sf$Fleet)
if(sum(is.na(sf$Region))) stop()
if(sum(is.na(sf$Method))) stop()
if(sum(is.na(sf$Year))) stop()
if(sum(is.na(sf$Quarter))) stop()

temp = subset(sf,Region=='EA'&Method=='OT')
cast(ddply(temp,.(Year,Gear),summarise,n=sum(TnoFish)),Year~Gear)

########################################
# Determine which size classes to keep (little point in keeping size classes
# with very few data)
sizes = sprintf('C%03d',1:150)
plot(colSums(sf[,sizes]))
abline(v=c(10,70))

# Only keep 20-80cm size classes
# (same as appears to have been used in previous assessments)
# Note that C001 is 10cm
keep = sprintf('C%03d',10:70)

# Aggregate by OM regions, methods, years and quarters
sums = ddply(sf,.(Region,Method,Year,Quarter),function(sub){
    colSums(sub[,keep])
})

# Calculate a total number of fish per stratum
sums$Num = rowSums(sums[,keep])

# Convert to proportions
sums[,keep] = sums[,keep]/sums$Num

# Some summaries of numbers measured and aggregate histos
hist(log10(sums$Num),breaks=100)
sums[order(sums$Num,decreasing=T),c('Region','Method','Year','Quarter','Num')]

ggplot(sums) + 
    geom_line(aes(x=Year+(Quarter-1)/4,y=Num,color=Method),alpha=0.5) + 
    geom_point(aes(x=Year+(Quarter-1)/4,y=Num,color=Method,shape=Method),size=3,alpha=0.5) +
    facet_wrap(~Region) + 
    scale_y_log10() + 
    labs(y='Number',x='Year')

par(mfrow=c(3,2))
for(method in c('PL','PS','GN','OT')) plot(20:80,colMeans(sums[sums$Method==method,keep]),main=method,ylab="",xlab="")

#################################
# Prepare data for outputting to model

# Only use samples where more than 100 fish
data = subset(sums,Num>100)
# Reshape and reorder
data = melt(data,id.vars=c('Year','Quarter','Region','Method','Num'))
# Rename columns
names(data) = c('year','quarter','region','method','count','size','proportion')
# Recode to match model coding
data = within(data,{
  # Convert dimensions to 0-based indexes
  quarter = quarter-1
  region = as.integer(region)-1
  method = as.integer(method)-1
  # Convert 1 mm size to an integer
  size = as.character(size)
  size = as.integer(substr(size,2,nchar(size)))
  # Need to add 10cm because C001 is 10cm
  size = size + 10
  # Create a 2mm size bin which is indexed as in model
  # e.g a 43mm fish is in size class 21 (21*2 = 42 = 42 to 44mm)
  class = floor(size/2)
  class[class<0] = NA # Can't have a class index <0
  class[class>=40] = NA # Can't have a class index >39
})
# Bin into 2mm size classes
data = ddply(subset(data,!is.na(class)),.(year,quarter,region,method,class),summarise,
  size = head(count,n=1),
  proportion = round(sum(proportion),6)
)
range(data$year)
range(data$quarter)
range(data$region)
range(data$method)
range(data$class)
# Reorder columns
data = data[,c('year','quarter','region','method','class','proportion','size')]
# Re-sort
data = data[with(data,order(year,quarter,region,method)),]
# Write
write.table(data,file='input/size_freqs.tsv',row.names=F,quote=F,sep='\t')


