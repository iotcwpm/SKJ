library(ggplot2)

safety <- 10
threshold <- 40
imax <- 100

values <- within(data.frame(status=seq(0,100,1)),{
  value <- imax/(threshold-safety)*(status-safety)
  value[status<safety] <- 0
  value[status>threshold] <- imax
})

plot <- ggplot(values) +
  geom_rect(xmin=0,xmax=40,ymin=0,ymax=100,fill=hsv(0.15,0.5,0.9)) +
  geom_rect(xmin=40,xmax=100,ymin=0,ymax=100,fill=hsv(0.35,0.5,0.9)) +
  geom_rect(xmin=0,xmax=40,ymin=100,ymax=200,fill=hsv(0,0.5,0.9)) +
  geom_rect(xmin=40,xmax=100,ymin=100,ymax=200,fill=hsv(0.15,0.5,0.9)) +
  geom_line(aes(x=status,y=value)) +
  geom_vline(aes(xintercept=safety),lty=2,col='#444444') + 
  annotate('text',x=safety,y=0,label=paste0('Safety level = ',safety,'%B0'),angle=90,vjust=-0.5,hjust=-0.1,col='#444444') +
  geom_vline(aes(xintercept=threshold),lty=2,col='#444444') + 
  annotate('text',x=threshold,y=0,label=paste0('Threshold level = ',threshold,'%B0'),angle=90,vjust=-0.5,hjust=-0.1,col='#444444') +
  geom_hline(aes(yintercept=imax),lty=2,col='#444444') +
  annotate('text',x=100,y=imax,label=paste0('Maximum fishing intensity = ',imax,'%Etarg'),vjust=-0.5,hjust=1.1,col='#444444') +
  scale_x_continuous('Stock status (%B0)',limits=c(0,100),expand=c(0,0)) +
  scale_y_continuous('Fishing intensity (%Etarg)',limits=c(0,150),expand=c(0,0)) +
  labs(colour="")

svg("maldives2016.svg", width=6, height=4)
#par(mar=c(5,5,3,3))
print(plot)
dev.off()

write.table(format(values,digits=2), 'maldives2016.txt', row.names=F, quote=F)

