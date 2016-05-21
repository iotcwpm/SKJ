library(ggplot2)

threshold <- 40
safety <- 10
basis <- 'e'
emax <- 0.3
cmax <- 600

mal2016 <- function(status,emax,threshold,safety){
  if(basis=='e'){
    value <- emax/(threshold-safety)*(status-safety)
    value[status<safety] <- 0
    value[status>threshold] <- emax
  } else {
    value <- cmax*((status-safety)/(threshold-safety))^2
    value[status<safety] <- 0
    value[status>threshold] <- cmax
  }
  value
}

values <- within(data.frame(status=seq(0,100,1)),{
  value <- mal2016(status,emax,threshold,safety)
  catch <- (value * status)/100
})

plot <- ggplot(values) +
  geom_rect(xmin=0,xmax=40,ymin=0,ymax=100,fill=hsv(0.15,0.5,0.9)) +
  geom_rect(xmin=40,xmax=100,ymin=0,ymax=100,fill=hsv(0.35,0.5,0.9)) +
  geom_rect(xmin=0,xmax=40,ymin=100,ymax=200,fill=hsv(0,0.5,0.9)) +
  geom_rect(xmin=40,xmax=100,ymin=100,ymax=200,fill=hsv(0.15,0.5,0.9)) +
  geom_line(aes(x=status,y=value)) +
  geom_vline(aes(xintercept=safety),lty=2,col='#444444') + 
  annotate('text',x=safety,y=0,label=paste('Safety limit (X) =',safety),angle=90,vjust=-0.5,hjust=-0.1,col='#444444') +
  geom_vline(aes(xintercept=threshold),lty=2,col='#444444') + 
  annotate('text',x=threshold,y=0,label=paste('Threshold level (T) =',threshold),angle=90,vjust=-0.5,hjust=-0.1,col='#444444') +
  geom_hline(aes(yintercept=emax),lty=2,col='#444444') +
  scale_x_continuous('Stock status (%B0)',limits=c(0,100),expand=c(0,0)) +
  labs(colour="")

if(basis=='i'){
  plot <- plot +
    annotate('text',x=100,y=emax,label=paste('Maximum fishing intensity (Imax) =',emax),vjust=-0.5,hjust=1.1,col='#444444') +
    scale_y_continuous('Fishing intensity (%Etarg)',limits=c(0,150),expand=c(0,0))
} else if(basis=='e'){
  plot <- plot +
    annotate('text',x=100,y=emax,label=paste('Maximum exploitation rate (Emax) =',emax),vjust=-0.5,hjust=1.1,col='#444444') +
    scale_y_continuous('Exploitation rate',limits=c(0,0.4),expand=c(0,0))
} else {
  plot <- plot +
    annotate('text',x=100,y=emax,label=paste('Maximum catch =',cmax),vjust=-0.5,hjust=1.1,col='#444444') +
    scale_y_continuous('Catch (`000 t)',limits=c(0,800),expand=c(0,0))  
}

if(basis=='c') {
  plot <- ggplot(values,aes(x=status,y=catch)) + 
    geom_line() + 
    labs(x='Status %B0',y='Relative catch (%)')
}

svg("mal2016.svg", width=6, height=4)
par(mar=c(5,5,3,3))
print(plot)
dev.off()

write.table(format(values,digits=2), 'mal2016.txt', row.names=F, quote=F)

