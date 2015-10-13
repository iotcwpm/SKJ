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

run <- function(pars){
	cat(toJSON(pars,pretty=T),file='parameters/input/parameters.json')
	system('./ioskj.exe run ref 0 0')
}