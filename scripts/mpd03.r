# A R script for generating plots for MPD03
source('evaluation.r')

plot_ribbon_catch_status(
  min(subset(procedures,class=="Mald2016" & p10=='ref')$procedure),
  start=1980,
  title=FALSE
)
plot_ribbon_catch_status(
  min(subset(procedures,class=="ConstCatch" & p1==700000)$procedure),
  start=1980,
  title=FALSE
)

multiplot(
  plot_whisker_mp_par(
    subset(performances, p10=='ref*imax'),
    y='catches_total', ylab="Yield (average annual catch, kt)",
    x='p5', xlab='Maximum fishing intensity (Imax)'
  ),
  plot_whisker_mp_par(
    subset(performances, p10=='ref*imax'),
    y='status_mean', ylab="Stock status (average SB/SB0)",
    x='p5', xlab='Maximum fishing intensity (Imax)'
  )
)

plot_tradeoff(
  subset(perfs, p10=='ref*imax'),
  x = list(status='Status (average SB/SB0)'),
  y = list(yield='Yield (average annual catch, kt)'),
  colour = list(p5='Imax'),
  shape = list(p5='Imax'),
  bars=F
)

plot_tradeoff(
  subset(perfs, class %in% c('ConstCatch','FRange','IRate','Mald2016')),
  x = list(status='Status (average SB/SB0)'),
  y = list(yield='Yield (average annual catch, kt)'),
  colour = list(class='Class'),
  shape = list(class='Class'),
  bars=T
)
