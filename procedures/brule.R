library(ggplot2)

imax <- 100
threshold <- 40
safety <- 10

brule <- function(status,imax,threshold,safety){
  intensity <- imax/(threshold-safety)*(status-safety)
  intensity[status<safety] <- 0
  intensity[status>threshold] <- imax
  intensity
}

values <- within(data.frame(status=seq(0,100,1)),{
  intensity <- brule(status,imax,threshold,safety)
})

plot <- ggplot(values) +
  geom_rect(xmin=0,xmax=40,ymin=0,ymax=100,fill=hsv(0.15,0.5,0.9)) +
  geom_rect(xmin=40,xmax=100,ymin=0,ymax=100,fill=hsv(0.35,0.5,0.9)) +
  geom_rect(xmin=0,xmax=40,ymin=100,ymax=200,fill=hsv(0,0.5,0.9)) +
  geom_rect(xmin=40,xmax=100,ymin=100,ymax=200,fill=hsv(0.15,0.5,0.9)) +
  geom_line(aes(x=status,y=intensity)) +
  geom_vline(aes(xintercept=safety),lty=2) + 
  annotate('text',x=safety,y=0,label=paste('Safety limit (X) =',safety),angle=90,vjust=-0.5,hjust=-0.1) +
  geom_vline(aes(xintercept=threshold),lty=2) + 
  annotate('text',x=threshold,y=0,label=paste('Threshold level (T) =',threshold),angle=90,vjust=-0.5,hjust=-0.1) +
  geom_hline(aes(yintercept=imax),lty=2,col='darkgrey') + 
  annotate('text',x=0,y=imax,label=paste('Maximum fishing intensity (Imax) =',imax),vjust=-0.5,hjust=0,col='#444444') +
  scale_x_continuous('Stock status (%B0)',limits=c(0,100),expand=c(0,0)) +
  scale_y_continuous('Fishing intensity (%Etarg)',limits=c(0,150),expand=c(0,0)) + 
  labs(colour="")
print(plot)
