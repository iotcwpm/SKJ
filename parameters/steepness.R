#!/usr/bin/Rscript

# Script for exploring prior on stock recruitment steepness

steepnesses = seq(0.2,1,0.01)

# Functions to r

beta_bounded = function(par1,par2){
  qs = (qbeta(c(0.05,0.2,0.5,0.8,0.95),par1,par2)+0.25)/1.25
  dens = dbeta((steepnesses*1.25)-0.25,par1,par2)
  list(
    qs = qs,
    dens = dens
  )
}

beta_unbounded = function(par1,par2){
  qs = qbeta(c(0.05,0.2,0.5,0.8,0.95),par1,par2)
  dens = dbeta(steepnesses,par1,par2)
  list(
    qs = qs,
    dens = dens
  )
}

# Find bounded beta parameters that provide the desired
# quantiles

fit = function(pars,target){
  qs = beta_bounded(pars[1],pars[2])$qs
  sum((qs-target)^2)
}

optimise = function(target=c(0.65,0.7,0.85,0.9,0.95)){
  optim(c(2,1),fit,target=target)$par
}

optimise()

# Plot comparision of priors

p1 = beta_bounded(8.5,2.5)
p2 = beta_unbounded(18,4)
colours = hsv(c(0.7,0),0.8,0.8,0.7)

svg("steepness.svg",width=7,height=4)
par(mar=c(4,4,1,1))
plot(steepnesses,p1$dens,type='l',ylab='Density',xlab='',col=colours[1],lwd=2,las=1,ylim=c(0,5))
lines(steepnesses,p2$dens,col=colours[2],lwd=2,lty=2)
mtext(1,text=expression(paste("Steepness (",eta,")")),line=3)
legend('topleft',legend=c('This study','Atlantic'),col=colours,lty=c(1,2),bty='n')
dev.off()

write.table(
  data.frame(p=c(0.05,0.2,0.5,0.8,0.95),eta=p1$qs),
  row.names=F,
  file="steepness.txt"
)

