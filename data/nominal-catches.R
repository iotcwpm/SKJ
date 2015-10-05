#!/usr/bin/Rscript

# A R script to process nominal catch data
# The sheet "Catch_Captures" from "IOTC-2015-WPTT17-DATA04-NC.xlsx" 
# was copied and pasted into file "source/IOTC-2015-WPTT17-DATA04-NC.txt"

library(reshape)
library(ggplot2)

# Read in data
nc <- read.table('source/IOTC-2015-WPTT17-DATA04-NC.txt',header=T,sep='\t')

# Assign to operatng model areas and methods and quarters
# Here we do not split western into SW and NW, or into
# quarters, that is done elsewhere
nc <- within(nc,{
    # Assign to an area based on area and fleet
    AreaOM = NA
    AreaOM[AreaIOTC=='Eastern Indian Ocean'] = 'EA'
    AreaOM[AreaIOTC=='Western Indian Ocean' & FlCde=='MDV'] = 'MA'
    AreaOM[AreaIOTC=='Western Indian Ocean' & FlCde!='MDV'] = 'W'
    AreaOM =factor(AreaOM,levels=c(
        'W','MA','EA'
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
write.table(sums,file='processed-data/nominal-catches.txt',row.names=F,quote=F,sep='\t')
