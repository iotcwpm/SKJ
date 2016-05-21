# A R script for generating plots for MPD03
source('evaluation.r')

plot_whisker_mp_par(
  performances,
  y='catches_total', ylab="Yield",
  x='class', xlab='Class'
)

plot_whisker_mp_par(
  subset(performances, p10=='ref*imax'),
  y='catches_total', ylab="Yield (average annual catch, kt)",
  x='p5', xlab='Maximum fishing intensity (Imax)'
)

plot_whisker_mp_par(
  subset(performances, p10=='ref*imax'),
  y='status_mean', ylab="Stock status (average SB/SB0)",
  x='p5', xlab='Maximum fishing intensity (Imax)'
)

plot_tradeoff(
  subset(perfs,  p10=='ref*imax'),
  x = list(status='Status (average SB/SB0)'),
  y = list(yield='Yield (average annual catch, kt)'),
  colour = list(class='Class'),
  shape = list(class='Class'),
  bars=F
)



