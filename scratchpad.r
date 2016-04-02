# A R script for comon commands to be run interactively

source('ioskj/ioskj.r')

read_track()

plot_track('biomass_status','Stock status (%B0)')
plot_track('catches_total','Catch (overall;t)')
plot_track('catch_fraction','Catch fraction')


# Maldives2016
# p1 = frequency
# p2 = precision
# p3 = emax
# p4 = thresh
# p5 = closure
# p6 = cmax



# Plot of tradeoff between two performance statistics
plot_tradeoff <- function(data,x,y,colour,shape){
  data <- within(data,{
    xm = data[,names(x)]
    xsd = data[,paste0(names(x),'_sd')]
    ym = data[,names(y)]
    ysd = data[,paste0(names(y),'_sd')]
    colour = factor(data[,names(colour)])
    shape = factor(data[,names(shape)])
  })
  print(
    ggplot(data,aes(colour=colour,shape=shape)) +
      geom_point(aes(x=xm,y=ym),size=3,alpha=0.7) + 
      geom_segment(aes(x=xm-xsd,xend=xm+xsd,y=ym,yend=ym),alpha=0.5) +
      geom_segment(aes(x=xm,xend=xm,y=ym-ysd,yend=ym+ysd),alpha=0.5) +
      geom_vline(xintercept=0,alpha=0) + 
      geom_hline(yintercept=0,alpha=0) + 
      scale_shape_manual(values=1:10) + 
      labs(x=x[[1]],y=y[[1]],colour=colour[[1]],shape=shape[[1]])
  )
}


#` Plot of the distributions of performance statistics
#` by parameter
plot_perf_distr <- function(data,x){
  data <- within(data,{
    xm <- data[,names(x)]
  })
  ggplot(data,aes(x=x)) + geom_violin() + 
    labs(x=x[[1]])
}

plot_perf_distr(
  subset(perfs,class=="Maldives2016"),
  x = list(p3='Emax'),  
)

plot_tradeoff(
  subset(perfs,class=="Maldives2016"),
  x = list(yield='Yield (mean annual catch, `000 t)'),
  y = list(status='Status (mean of B/B0)'),
  colour = list(p3='Emax'),
  shape = list(p4='Threshold')
)

plot_tradeoff(
  subset(perfs,class=="Maldives2016"),
  x = list(yield='Yield (mean annual catch, `000 t)'),
  y = list(safety20='Safety'),
  colour = list(p3='Emax'),
  shape = list(p4='Threshold')
)

