#if(!require(jsonlite)){
#	install.packages('jsonlite')
#	require(jsonlite)
#}
library(ggplot2)
library(dplyr)

Fixed <- function(value){
	list(value=unbox(value))
}

Uniform <- function(value,lower,upper){
	list(value=unbox(value),lower=unbox(lower),upper=unbox(upper))
}

Normal <- function(value,mean,sd){
	list(value=unbox(value),mean=unbox(mean),sd=unbox(sd))
}

TruncatedNormal <- function(value,mean,sd,min,max){
	list(value=unbox(value),mean=unbox(mean),sd=unbox(sd),min=unbox(min),max=unbox(max))
}

Lognormal <- function(value,location,dispersion){
	list(value=unbox(value),location=unbox(location),dispersion=unbox(dispersion))
}

Beta <- function(value,alpha,beta){
	list(value=unbox(value),alpha=unbox(alpha),beta=unbox(beta))
}


run <- function(pars){
	cat(toJSON(pars,pretty=T),file='parameters/input/parameters.json')
	system('./ioskj.exe run ref 0 0')
}


track <<- NULL
perfs <<- NULL
evaluate <- function(procedures,replicates=1,samples_file="feasible/output/accepted.tsv",year_start=-1,dir='.'){
	if(dir!='.'){
		cwd <- getwd()
		setwd(dir)
	}

	write.table(procedures,row.names=F,col.names=T,quote=F,file="procedures/input/procedures.tsv")
	system(paste('./ioskj.exe evaluate_wrap',replicates,samples_file,year_start))

	if(dir!='.') setwd(cwd)

	track <<- read.table('ioskj/evaluate/output/track.tsv',header=T)
	track <<- track %>% group_by(replicate,procedure,year) %>% summarise(
		biomass_status = head(biomass_status,1)*100,
		catches_total = sum(catches_total)/1000
	)
	
	perfs <<- read.table('ioskj/evaluate/output/performances.tsv',header=T)
}

plot_track <- function(what,label){
	plot <- ggplot(track,aes(x=year,color=procedure,group=paste(replicate,procedure))) + 
		geom_line(aes_string(y=what),alpha=0.5) + 
		geom_hline(yintercept=0,alpha=0) +
		geom_vline(xintercept=2015,alpha=0.4,linetype=2) +
		labs(x='Year',y=label,color='Proc.')

	print(plot)
	plot
}

plot_track_ribbons <- function(what,label){

	quantiles <- track %>% group_by(procedure,year) %>% do({
		qs <- quantile(as.data.frame(.)[,what],p=c(0.05,0.1,0.25,0.75,0.9,0.95))
		qs <- as.data.frame(matrix(qs,nrow=1))
		names(qs) <- paste0(what,'_q',1:6)
		qs
	})

	plot <- ggplot(quantiles,aes(x=year,fill=procedure)) + 
		geom_ribbon(aes_string(
			ymin=paste0(what,'_q1'),
			ymax=paste0(what,'_q6')
		),alpha=0.2) + 
		geom_ribbon(aes_string(
			ymin=paste0(what,'_q2'),
			ymax=paste0(what,'_q5')
		),alpha=0.2) + 
		geom_ribbon(aes_string(
			ymin=paste0(what,'_q3'),
			ymax=paste0(what,'_q4')
		),alpha=0.2) + 
		geom_hline(yintercept=0,alpha=0) +
		geom_vline(xintercept=2015,alpha=0.6,linetype=2) +
		labs(x='Year',y=label)

	print(plot)
	plot
}

brule <- function(s,target,threshold,limit){
  f <- target/(threshold-limit)*(s-limit)
  f[s<limit] <- 0
  f[s>threshold] <- target
  f
}


