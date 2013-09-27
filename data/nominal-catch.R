#!/usr/bin/Rscript

# A R script to process nominal catch data

library(reshape)
library(ggplot2)

# Read in data
nc = read.table('source-data/NCTROP.txt',header=T,sep='\t')

# Assign to operatng model areas and methods and quarters
nc = within(nc,{
    # Assign to an area based on Area and Fleet
    # use unique(nc[,c('Fleet','EName')]) to get a Fleet name
    AreaOM = NA
    AreaOM[Area=='IO_Eastern'] = 'E'
    AreaOM[Area=='IO_Western' & Fleet=='MDV'] = 'M'
    AreaOM[Area=='IO_Western' & Fleet!='MDV'] = 'W'
    AreaOM =factor(AreaOM,levels=c(
        'W','M','E'
    ))
    
    # Assign to a method
    Method = NA
    Method[GearGroup=='Purse Seine'] = 'PS'
    Method[GearGroup=='Baitboat'] = 'PL'
    Method[GearGroup=='Gillnet'] = 'GN'
    Method[GearGroup=='Line'] = 'LI'
    # Longline has very minor catches in all areas so inclue in other 
    Method[GearGroup=='Other' | GearGroup=='OTHER' | GearGroup=='Longline'] = 'OT'
    Method =factor(Method,levels=c(
        'PS','PL','GN','LI','OT'
    ))

    # Assign to a quarter
    Quarter = NA
})

# Aggregate by OM areas, methods, years and quarters
sums = ddply(nc,.(AreaOM,Method,Year,Quarter),summarise,Catch=sum(SKJ,na.rm=T))

# Plot
ggplot(sums) + 
    geom_line(aes(x=Year,y=Catch,color=Method),alpha=0.5) + 
    geom_point(aes(x=Year,y=Catch,color=Method,shape=Method),size=3,alpha=0.5) +
    facet_wrap(~AreaOM) + 
    labs(y='Catch (t)')

# Rename columns and output
names(sums) = c('area','method','year','quarter','catch')
write.table(sums,file='processed-data/nominal-catch.txt',row.names=F,quote=F,sep='\t')
