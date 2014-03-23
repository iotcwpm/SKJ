# A general settings script for loading packages, setting themes etc

require(ggplot2)
require(plyr)

theme_set(theme_bw())
theme_update(
  panel.border = element_rect(fill = NA, colour = 'black'),
  panel.grid.major = element_line(colour='white',size=0.1),
  panel.grid.minor = element_line(colour='white',size=0.1),
  strip.background = element_rect(fill = 'grey80', colour = 'black'),
  axis.text.x = element_text(size=11),
  axis.text.y = element_text(size=11),
  axis.title.x = element_text(size=11,vjust = 0),
  axis.title.y = element_text(size=11,vjust = 0.5,angle = 90),
  legend.key = element_rect(colour = 'white'),
  legend.title = element_text(size=11,hjust = 0)
)

#' Convert model dimensions into factors (or another integer in the case of 'size') in a data.frame
dimensions <- function(data){
  for(name in names(data)){
    # Replace the size dimensions with length
    if(name=='size'){
      data$length <- data$size*2+1
      data$size <- NULL
    }
    else if(name=='size_from'){
      data$length_from <- data$size_from*2+1
      data$size_from <- NULL
    }
    else {
      # Set factors for other dimensions
      levels = switch(name,
        region = c('W','M','E'),
        region_from = c('W','M','E'),
        method = c('PS','PL','GN','LI','OT'),
        z_size = c('45-50','50-55','55-60','60-65'),
        NULL
      )
      if(!is.null(levels)) data[,name] <- factor(levels[data[,name]+1],levels=levels)
    }
  }
  data
}

#' Function to load a set of files that have been written by C++ to the output directory,
#' and assign them to workspace variables
load <- function(names){
  for(name in names){
    # Read in data
    data = read.table(paste("output/",name,".tsv",sep=''),header=T)
    # Convert dimensions into factors
    data = dimensions(data)
    # Assign the variable
    assign(name,data,env=parent.frame())
  }
}