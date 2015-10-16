if(!require(jsonlite)){
	install.packages('jsonlite')
	require(jsonlite)
}

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