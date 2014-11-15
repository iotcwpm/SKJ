#!/usr/bin/Rscript

# A R script to process catch-effort data files
# This is currently not used since nominal-catch-data.R is used to 
# create a catch history.
# The catch-efort data has more spatial detail but is incomplete

library(reshape)
library(ggplot2)
library(maps)

############################################################
# Load and bind
#
# The files do not have the same fields so this section
# extracts the relevant SKJ fields and binds the data together

# All files share the same core fields...
fields = c("Fleet","Gear","Year","MonthStart","MonthEnd","iGrid","Grid","Effort","EffortUnits","QualityCode","Source")

# CEPSBB.csv
psbb = read.csv('source/CEALL/CEPSBB.csv',header=T)
# Relevant fields are:
#   SKJ-FS    Catches of skipjack tuna recorded under free schools
#   SKJ-LS	Catches of skipjack tuna recorded under associated schools
# This file also has a field called "CatchUnits" but CE_Reference.xlsx
# says all in tonnes" as confiemed by unique(psbb$CatchUnits)
psbb = psbb[,c(fields,'SKJ.FS','SKJ.LS')]
# Melt out FS/LS division, rename and recode...
psbb = melt(psbb,id.vars=fields)
names(psbb)[12:13] = c('School','Catch')
psbb = within(psbb,{
    School = as.character(School)
    School[School=='SKJ.FS'] = 'FS'
    School[School=='SKJ.LS'] = 'LS'
})
# Restrict to positive catches
psbb = psbb[!is.na(psbb$Catch),]

# CELL.csv
ll = read.csv('source/CEALL/CELL.csv',header=T)
# Relevant fields are:
#   SKJ-NO    Catches of skipjack tuna recorded in number of specimens
#   SKJ-MT	Catches of skipjack tuna recorded in weight (tonnes)
ll = ll[,c(fields,'SKJ.MT')]
# Rename and restrict
names(ll)[12] = 'Catch'
ll = ll[!is.na(ll$Catch),]
# Add in null for School
ll$School = NA

# CEOTHR.csv
other = read.csv('source/CEALL/CEOTHR.csv',header=T)
# Relevant fields are:
#   SKJ-NO    Catches of skipjack tuna recorded in number of specimens
#   SKJ-MT	Catches of skipjack tuna recorded in weight (tonnes)
other = other[,c(fields,'SKJ.MT')]
# Rename and restrict
names(other)[12] = 'Catch'
other = other[!is.na(other$Catch),]
# Add in null for School
other$School = NA

# Bind all together
ce = rbind(rbind(psbb,ll),other)

# The field iGrid appears to be the same as Grid
# and is undocumented in CE_Reference.xlsx so drop it
ce = ce[,names(ce)!='iGrid']

############################################################
# Normalise resolution
#
# There are alternative resolutions for MonthStart/MonthEnd 
# and Grid. 

# Only three records have MonthStart!=MonthEnd (they are annual data in 1991):
subset(ce,MonthStart!=MonthEnd)
# These are for Indonesian exploratory longline fishing and only represent <0.01% of catch 
sum(subset(ce,MonthStart!=MonthEnd)$Catch)/sum(ce$Catch)*100
# So just drop them, get rid of MonthEnd and rename MonthStart to Month
ce = subset(ce,MonthStart==MonthEnd)
ce = ce[,names(ce)!='MonthEnd']
names(ce)[names(ce)=='MonthStart'] = 'Month'

# Decompose Grid
ce = within(ce,{
    GridSize = as.integer(substr(as.character(Grid),1,1))
    GridQuadrant = as.integer(substr(as.character(Grid),2,2))
    GridLat = as.integer(substr(as.character(Grid),3,4))
    GridLon = as.integer(substr(as.character(Grid),5,7))
    
    Lat = ifelse(GridQuadrant %in% c(1,4),1,-1) * (GridLat + c(2.5,5,5,10,0.5,2.5)[GridSize]) 
    Lon = ifelse(GridQuadrant %in% c(1,2),0,180) + (GridLon + c(5,10,5,10,0.5,2.5)[GridSize])
})

unique(ce$GridSize)

# Cells of 10x20 (GridSize==2)
unique(subset(ce,GridSize==2,select=c('GridLat','GridLon')))

# Cells of 5x5 (GridSize==6)
unique(subset(ce,GridSize==6,select=c('GridLat','GridLon')))

# Cells of 1x1 (GridSize==5)
temp = ddply(subset(ce,GridSize==5),.(Lat,Lon),summarise,Catch=sum(Catch))
areas = read.table('area-polygons.txt',header=T)
ggplot() + 
    geom_polygon(data=areas,aes(x=Lon,y=Lat,group=Area),colour='red',fill=NA) + 
    geom_polygon(data=map_data('world'),aes(x=long,y=lat,group=group),colour='grey70') + 
    geom_point(data=temp,aes(x=Lon,y=Lat,size=Catch),colour='blue',alpha=0.3) + 
    coord_map(xlim=c(20,160),ylim=c(-50,40)) + 
    scale_area()

