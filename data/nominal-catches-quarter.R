#!/usr/bin/Rscript

# A R script to process nominal catch data

library(reshape)
library(ggplot2)

# Read in data
nc = read.table('source/CAS_Tables_SKJ_2013.txt',header=T,sep='\t',as.is=T)
nc$tmt = as.numeric(nc$tmt)

# Assign to operating model regions
nc = within(nc,{
  # Assign to an region based on IOTC_Area and Fleet
  # use unique(nc[,c('Fleet','EName')]) to get a Fleet name
  Region = NA
  Region[IOTC_Area=='IO_Eastern'] = 'E'
  Region[IOTC_Area=='IO_Western' & Fleet=='MDV'] = 'M'
  Region[IOTC_Area=='IO_Western' & Fleet!='MDV'] = 'W'
  Region = factor(Region,levels=c(
    'W','M','E'
  ))
})

# Summary to check above method coding
cast(ddply(nc,.(Region,Gear),summarise,catch=sum(tmt)),Gear~Region)
ggplot(ddply(nc,.(Region,Gear,Year),summarise,catch=log10(sum(tmt)))) + 
  geom_line(aes(x=Year,y=catch,colour=Gear,shape=Gear)) + 
  geom_point(aes(x=Year,y=catch,colour=Gear,shape=Gear),size=3) + 
  scale_shape_manual(values=1:27) +
  facet_wrap(~Region)

# Assign to operating model methods
nc = within(nc,{
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
sums = ddply(nc,.(Region,Method,Year,Quarter),summarise,Catch=sum(tmt,na.rm=T))

# Plot
svg("nominal-catches-quarter.svg",width=25/2.54,height=18/2.54)
ggplot(sums) + 
  geom_line(aes(x=Year,y=Catch,color=Method),alpha=0.5) + 
  geom_point(aes(x=Year,y=Catch,color=Method,shape=Method),size=3,alpha=0.5) +
  facet_wrap(~Region) + 
  labs(y='Catch (t)')
dev.off()

svg("nominal-catches-quarter-annual.svg",width=25/2.54,height=18/2.54)
ggplot(ddply(nc,.(Region,Method,Year),summarise,Catch=sum(tmt,na.rm=T))) + 
  geom_line(aes(x=Year,y=Catch,color=Method),alpha=0.5) + 
  geom_point(aes(x=Year,y=Catch,color=Method,shape=Method),size=3,alpha=0.5) +
  facet_wrap(~Region) + 
  labs(y='Catch (t)')
dev.off()

# Rename columns, reorder, sort, recode and output for reading into model
names(sums) = c('region','method','year','quarter','catch')
sums = sums[,c('year','quarter','region','method','catch')]
sums = sums[with(sums,order(year,quarter,region,method)),]
sums = within(sums,{
  quarter = quarter-1
  region = as.integer(region)-1
  method = as.integer(method)-1
})
write.table(sums,file='nominal-catches.tsv',row.names=F,quote=F,sep='\t')

# Calculate catch proportions by region/method for last 10 years
temp <- ddply(subset(sums,year>max(sums$year)-10),.(region,method),summarise,catch=sum(catch))
temp$value <- with(temp,round(catch/sum(catch),3))
temp <- merge(expand.grid(region=0:2,method=0:3),temp,all.x=T)
temp$catch <- NULL
temp$value[is.na(temp$value)]<- 0
write.table(temp,file='catches-allocations.tsv',row.names=F,quote=F,sep='\t')


