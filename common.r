# A general settings script for loading packages, setting themes etc

require(ggplot2)
require(plyr)
require(reshape)

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
    # Give length for size dimensions
    if(name=='size'){
      data$length <- data$size*2+1
    }
    else if(name=='size_from'){
      data$length_from <- data$size_from*2+1
    }
    else {
      # Set factors for other dimensions
      levels = switch(name,
        region = c('SW','NW','MA','EA'),
        region_from = c('SW','NW','MA','EA'),
        method = c('PS','PL','GN','OT'),
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
load <- function(names,from){
  for(name in names){
    # Read in data
    data = read.table(file.path(from,paste0(name,".tsv")),header=T,sep='\t')
    # Convert dimensions into factors
    data = dimensions(data)
    # Assign the variable
    assign(name,data,env=parent.frame())
  }
}

#' Function for doing a multiplot with ggplot
#' From http://www.cookbook-r.com/Graphs/Multiple_graphs_on_one_page_(ggplot2)/
multiplot <- function(..., plotlist=NULL, file, cols=1, layout=NULL) {
	require(grid)
	
	# Make a list from the ... arguments and plotlist
	plots <- c(list(...), plotlist)
	
	numPlots = length(plots)
	
	# If layout is NULL, then use 'cols' to determine layout
	if (is.null(layout)) {
		# Make the panel
		# ncol: Number of columns of plots
		# nrow: Number of rows needed, calculated from # of cols
		layout <- matrix(seq(1, cols * ceiling(numPlots/cols)),
						 ncol = cols, nrow = ceiling(numPlots/cols))
	}
	
	if (numPlots==1) {
		print(plots[[1]])
		
	} else {
		# Set up the page
		grid.newpage()
		pushViewport(viewport(layout = grid.layout(nrow(layout), ncol(layout))))
		
		# Make each plot, in the correct location
		for (i in 1:numPlots) {
			# Get the i,j matrix positions of the regions that contain this subplot
			matchidx <- as.data.frame(which(layout == i, arr.ind = TRUE))
			
			print(plots[[i]], vp = viewport(layout.pos.row = matchidx$row,
											layout.pos.col = matchidx$col))
		}
	}
}
