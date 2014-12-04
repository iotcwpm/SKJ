source('../common.R')

library(r4ss)
library(stringr)

#' Get a data.frame of runs with directory name and altered
#' options and parameter values parsed from name
runs <-  within(data.frame(
	dir = paste0('runs/',gsub('/ss3.par','',list.files('runs',pattern='ss3.par',recursive=T))),
	stringsAsFactors = F
),{
	opts <- as.factor(str_match(dir,'runs/(.+)/')[,2])
	m <- as.numeric(str_match(dir,'M(\\d\\.\\d)')[,2])
	h <- as.numeric(str_match(dir,'steepness(\\d\\.\\d)')[,2])
	ps <- as.factor(str_match(dir,'PS(\\d)')[,2])
})

#' For each run read in SS3 output and store in a list
models <-list()
for(index in 1:nrow(runs)){
	run <- runs[index,]
	print(run$dir)
	# Read in model output
	model <- SS_output(
		run$dir,
		covarfile = FALSE, 
		forefile = FALSE, 
		forecast = FALSE, 
		covar = FALSE
	)
	models[[run$dir]] <- model
}

#' Extract parameters from each model
pars <- NULL
for(index in 1:nrow(runs)){
	run <- runs[index,]
	print(run$dir)
		
	model <- models[[run$dir]]
	names <- NULL
	values <- NULL
	
	#' Unfished spawning biomass
	#' Use this instead of R0 because it is not dependent upon assumed
	#' size/age of recruits or early mortality schedule
	b0 <- model$timeseries[1,'SpawnBio']
	names <- c(names,'spawners_unfished.value')
	values <- c(values,b0)
	
	#' Recruitment deviations
	temp <- model$parameters[,c('Label','Value')]
	recdevs <- within(temp[grepl('Main_RecrDev',temp$Label),],{
		year <- as.numeric(str_match(Label,'Main_RecrDev_(\\d+)')[,2])
	})
	# Ensure the correct number of years of recdevs
	recdevs <- merge(data.frame(year=1985:2010),recdevs,all.x=T)
	recdevs$Value[is.na(recdevs$Value)] <- 0
	
	names <- c(names,paste0('recruits_deviations(',recdevs$year,').value'))
	values <- c(values,recdevs$Value)
		
	#' Selectivity for each method (a.k.a. gear, Fleet)
	#' Use the model$selex data.frame which gives calculated values which can be used
	#' for the knots in ioskj.
	#' Fleets:
	#' 	1 = PL
	#' 	2 = PSLS
	#' 	3 = PSFS
	#' 	4 = Other
	#' Fleets 5 & 6 are repeats of PL and PSFS for fitting CPUE
	selects <- subset(model$sizeselex,Factor=='Lsel' & year==2013 & Fleet %in% c(1,2,4))
	# Map fleets into ioskj method codes:
	#	PS = 0,
	#	PL = 1,
	#	GN = 2,
	#	OT = 3
	selects <- within(selects,{
		method <- NA
		method[Fleet==2] <- 0 # PSFS -> PS
		method[Fleet==1] <- 1 # PL -> PL
		method[Fleet==4] <- 3 # Other -> OT
	})
	# Repeat other row for gill net
	selects <- rbind(selects,within(subset(selects,method==3),method<-2))
	# Drop uneeded columns
	selects$Factor <- NULL
	selects$Fleet <- NULL
	selects$year <- NULL
	selects$gender <- NULL
	selects$label <- NULL
	# Reshape
	selects <- melt(selects,id.vars="method")
	selects$variable <- as.numeric(as.character(selects$variable))
	# Interpolate selectivity at length of the knots used in ioskj
	# Create parameter names and values
	knots <- 0:6
	for(meth in unique(selects$method)){
		names <- c(names,paste0('selectivities(',meth,',',knots,').value'))
		values <- c(values,with(subset(selects,method==meth),round(approx(x=variable,y=value,(knots*10)+20)$y,6)))
	}
	
	pars_ <- as.data.frame(matrix(values,nrow=1))
	names(pars_) <- names
	pars <- rbind(pars,pars_)
}
# Insert the fixed M and h
pars$mortality_base.value <- runs$m
pars$recruits_steepness.value <- runs$h

# Write to file
write.table(runs,file='runs.tsv',col.names=T,row.names=F,quote=F,sep='\t')
write.table(pars,file='pars.tsv',col.names=T,row.names=F,quote=F,sep='\t')

# Calculate some median parameter values for use in a reference set
pars <- read.table('pars.tsv',header=T,check.names=F)
pars <- melt(pars)
# Calculate medians
medians <- ddply(pars,.(variable),summarise,value=median(value))
# Calculate medians for recdev only if estimated
medians[substr(medians$variable,1,12)=="recruits_dev",] <- ddply(
	subset(pars,substr(variable,1,12)=="recruits_dev" & value!=0),
	.(variable),summarise,value=median(value)
)
# Output
write.table(
	medians,
	file='par-medians.tsv',
	col.names=T,row.names=F,quote=F,sep='\t'
)
