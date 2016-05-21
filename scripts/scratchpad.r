# A R script for comon commands to be run interactively
source('evaluation.r')

# ConstEffort
plot_ribbon_catch_status(
  subset(procedures,class=="ConstEffort" & p1==100)$procedure
)

# Illustration of ConstCatch with alternative levels
# and comparison with Mald2016
plot_ribbon_catch_status(
  subset(procedures,procedure<=10 & class=="ConstCatch" & p1==700000)$procedure
)
plot_ribbon_catch_status(
  subset(procedures,procedure<=10 & class=="ConstCatch" & p1==250000)$procedure
)

# Maldives2016
# p1 = frequency
# p2 = precision
# p3 = thresh
# p4 = closure
# p5 = imax
# p6 = cmax
# p7 = dmax
# p8
# p9
# p10 = tag

plot_ribbon_catch_status(
  min(subset(procedures,class=="Mald2016" & p10=='ref')$procedure)
)

plot_whisker_mp_par(
  performances,
  y='catches_total', ylab="Yield",
  x='class', xlab='Class'
)

plot_whisker_mp_par_multi(
  subset(performances,p10=='ref*imax'),
  x='p5', xlab='Maximum fishing intensity (Imax)', xrefs=1
)

plot_whisker_mp_par_multi(
  subset(performances,p10=='ref*thresh'),
  x='p3', xlab='Threshold level (T, B/B0)', xrefs=0.4
)

plot_whisker_mp_par_multi(
  subset(performances,p10=='ref*closure'),
  x='p4', xlab='Closure level (C, B/B0)', xrefs=0.1
)

plot_whisker_mp_par_multi(
  subset(performances,p10=='ref*dmax'),
  x='p7', xlab='Maximum change in recommended catch (Dmax)', xrefs=0.3
)


temp <- table_stat_summary(
  subset(performances,class=='Mald2016' & p10=='ref')
)
write.table(format(temp,digits=3),file='temp.txt',quote=F,row.names=F,sep='\t')







sub = subset(perfs,p10=='ref*imax')
ggplot(sub,aes(x=p6)) +
  geom_point(aes(y=yield))
ggplot(sub,aes(x=p6)) +
  geom_point(aes(y=safety20))
ggplot(sub,aes(x=p6)) +
  geom_point(aes(y=cl))


ggplot(subset(perfs,p8<10000),aes(x=p8)) +
  geom_point(aes(y=safety20))

# Tradeoff plots

plot_tradeoff(
  perfs,
  x = list(status='Status (average SB/SB0)'),
  y = list(yield='Yield (average annual catch, kt)'),
  colour = list(class='Class'),
  shape = list(class='Class'),
  bars=F
)

plot_tradeoff(
  subset(perfs,class=="Mald2016" | class=="ConstEffort"),
  x = list(status='Status (mean SB/SB0)'),
  y = list(yield='Yield (mean annual catch, kt)'),
  colour = list(class='Class'),
  shape = list(class='Class'),
  bars=F
)

plot_tradeoff(
  subset(perfs,class=="Mald2016" | class=="ConstEffort"),
  x = list(safety20='Safety'),
  y = list(yield='Yield (mean annual catch, kt)'),
  colour = list(class='Class'),
  shape = list(class='Class'),
  xmin=0.75, bars=F
)

plot_tradeoff(
  subset(perfs,class=="Mald2016"),
  x = list(status='Status (mean SB/SB0)'),
  y = list(yield='Yield (mean annual catch, kt)'),
  colour = list(p7='Cmax'),
  shape = list(p3='Theshold'),
  bars=F,xmin=50,ymin=400
) + geom_vline(xintercept=c(20,40),linetype=2,alpha=0.3)

plot_tradeoff(
  subset(perfs,class=="Mald2016"),
  x = list(kobe_a='Status P(Kobe green)'),
  y = list(yield='Yield (mean annual catch, kt)'),
  colour = list(p6='Imax'),
  shape = list(p3='Theshold'),
  xmin=0.5
)

plot_tradeoff(
  subset(perfs,class=="Mald2016"),
  x = list(safety20='Safety P(SB>0.2B0)'),
  y = list(yield='Yield (mean annual catch, kt)'),
  colour = list(p6='Imax'),
  shape = list(p3='Theshold'),
  xmin=0.5
)

plot_tradeoff(
  subset(perfs,class=="Mald2016"),
  x = list(stability='Stability'),
  y = list(yield='Yield (mean annual catch, kt)'),
  colour = list(p6='Imax'),
  shape = list(p3='Theshold'),
  xmin=-10
)



temp <- subset(perfs,class=="Mald2016" & p1==3 & p3==0.4 & p4==0.1 & p7==700000)

ggplot(temp,aes(x=p6,y=yield)) + 
  geom_point(size=5) +
  geom_errorbar(aes(ymin=yield-yield_sd, ymax=yield+yield_sd),width=0.01) +
  geom_hline(yintercept=0,alpha=0) +
  labs(x='Emax',y='Yield (t)')

ggplot(temp,aes(x=p6,y=status)) + 
  geom_point(size=5)+
  geom_errorbar(aes(ymin=status-status_sd, ymax=status+status_sd),width=0.01) +
  geom_hline(yintercept=0.4,linetype=2) +
  labs(x='Emax',y='Status (%B0)')

ggplot(temp,aes(x=p6,y=safety20)) + 
  geom_point(size=5) +
  geom_hline(yintercept=0.5,alpha=0) +
  labs(x='Emax',y='Safety P(B>20%B0)')

ggplot(temp,aes(x=p6,y=stability*100)) + 
  geom_point(size=5) +
  geom_hline(yintercept=0,alpha=0) +
  labs(x='Emax',y='Stability (% var catch limit)')



source('ioskj/ioskj.r')

read_track()

plot_track('biomass_status','Stock status (%B0)')
plot_track('catches_total','Catch (overall;t)')
plot_track('catch_fraction','Catch fraction')





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
  subset(perfs,class=="Mald2016"),
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

ggplot(references) +
  geom_density(aes(x=e_40),fill='red',alpha=0.3,adjust=0.5) +
  geom_density(aes(x=e_msy),fill='blue',alpha=0.3,adjust=0.5)

ggplot(references) +
  geom_density(aes(x=e_40*4),fill='red',alpha=0.3,adjust=0.5) +
  geom_density(aes(x=e_msy*4),fill='blue',alpha=0.3,adjust=0.5)

