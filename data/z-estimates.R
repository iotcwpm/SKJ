#!/usr/bin/Rscript

# A R script to process the z-estimates from Hillary & Everson (in press)
#
# Richard provided Z estimates (MLE [Z.mu] and standard errors [Z.sd]) for fully mixed and 
# purse seine only mixed reporting rate options.  
# Richard suggested using the purse seine only option, only 2005.75 to 2009.5 (due to time to get full mixing),
# and the 4 length classes 45,50,55,60

# Time frames are quarters from 2005.25 to 2009.50
times = seq(2005.25,2009.50,0.25)
# Size classes are:  20-30, 30-40, 40-45, 45-50, 50-55, 55-60, 60-65, 65-70, 70-75, 75-85
classes = c(20,30,40,45,50,55,60,65,70,75)

# Load estimates
load('source-data/SKJ_Z_psmixed.RData')

# Assign quarters and size class labels...

mu = within(as.data.frame(Z.mu),{
    time = times
    year = as.integer(time)
    quarter = (time-year)*4+1
})
names(mu)[1:10] = paste('mu',classes,sep='')

sd = within(as.data.frame(Z.sd),{
    time = times
    year = as.integer(time)
    quarter = (time-year)*4+1
})
names(sd)[1:10] = paste('sd',classes,sep='')

# Merge
zs = merge(mu,sd,by=c('year','quarter','time'))
zs = zs[order(zs$time),]

# A plot to check against Hillary & Everson's Figure 5
# magenta triangles
with(zs,{
    plot(mu60~time,ylim=c(0,1.75))
    segments(time,mu60-sd60*1.98,time,mu60+sd60*1.98)
})

# Restrict to times and sizes with reasonable CVs
zs = subset(zs,time>=2005.75 & time<=2009.5,select=c('year','quarter',paste('mu',c(45,50,55,60),sep=''),paste('sd',c(45,50,55,60),sep='')))

# Output
write.table(zs,file="processed-data/z-estimates.txt",row.names=F,quote=F,sep='\t')

