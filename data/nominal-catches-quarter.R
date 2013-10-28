#!/usr/bin/Rscript

# A R script to process nominal catch data

library(reshape)
library(ggplot2)

# Read in data
nc = read.table('source-data/CAS_Tables_SKJ_2013.txt',header=T,sep='\t',as.is=T)

# Assign to operatng model areas and methods and quarters
nc = within(nc,{
  # Assign to an area based on Area and Fleet
  # use unique(nc[,c('Fleet','EName')]) to get a Fleet name
  Area = NA
  Area[IOTC_Area=='IO_Eastern'] = 'E'
  Area[IOTC_Area=='IO_Western' & Fleet=='MDV'] = 'M'
  Area[IOTC_Area=='IO_Western' & Fleet!='MDV'] = 'W'
  Area = factor(Area,levels=c(
    'W','M','E'
  ))
  
  # Assign to a method
  # Based on GearGroup defined in GearCode sheet of NCTROP.xlsx
  Method = NA
  Method[Gear %in% c('PS','PSS','RIN')] = 'PS'
  Method[Gear %in% c('BB','BBM','BBN')] = 'PL'
  Method[Gear %in% c('GILL','GIOF','GL')] = 'GN'
  Method[Gear %in% c('HAND','LLCO','SPOR','TROL','TROLM','TROLN')] = 'LI'
  # Longline has very minor catches in all areas so inclue in other 
  Method[is.na(Method)] = 'OT'
  Method =factor(Method,levels=c(
    'PS','PL','GN','LI','OT'
  ))
  
  tmt = as.numeric(tmt)
})

# Aggregate by OM areas, methods, years and quarters
sums = ddply(nc,.(Area,Method,Year,Quarter),summarise,Catch=sum(tmt,na.rm=T))

# Plot
svg("nominal-catches-quarter.svg",width=25/2.54,height=18/2.54)
ggplot(sums) + 
  geom_line(aes(x=Year,y=Catch,color=Method),alpha=0.5) + 
  geom_point(aes(x=Year,y=Catch,color=Method,shape=Method),size=3,alpha=0.5) +
  facet_wrap(~Area) + 
  labs(y='Catch (t)')
dev.off()

svg("nominal-catches-quarter-annual.svg",width=25/2.54,height=18/2.54)
ggplot(ddply(nc,.(Area,Method,Year),summarise,Catch=sum(tmt,na.rm=T))) + 
  geom_line(aes(x=Year,y=Catch,color=Method),alpha=0.5) + 
  geom_point(aes(x=Year,y=Catch,color=Method,shape=Method),size=3,alpha=0.5) +
  facet_wrap(~Area) + 
  labs(y='Catch (t)')
dev.off()

# Rename columns and output
names(sums) = c('area','method','year','quarter','catch')
write.table(sums,file='processed-data/nominal-catches.txt',row.names=F,quote=F,sep='\t')
