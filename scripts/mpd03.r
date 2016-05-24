# A R script for generating plots for MPD03 presentation
source('common.r')
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

# Additional summaries as requested using evaluations with only
# three MPs

performances <- within(performances,{
  class <- as.character(class)
  class[class=='Mald2016'] <- 'BRule'
})







plot_ribbon_catch_status(
  subset(procedures,class=="ConstEffort")$procedure,
  start=1980, title=FALSE
)

plot_ribbon_catch_status(
  subset(procedures,class=="FRange")$procedure,
  start=1980, title=FALSE
)

plot_ribbon_catch_status(
  subset(procedures,class=="Mald2016")$procedure,
  start=1980, title=FALSE
)

table_stat_summary_full(
  subset(performances, class=='ConstEffort')
)

table_stat_summary_full(
  subset(performances, class=='FRange')
)

write.table(table_stat_summary_full(
  subset(performances, class=='FRange')
),'temp.txt',sep='\t')



plot_whisker_mp_par(
  performances,
  y='catches_total', ylab="Yield (average annual catch, kt)",
  x='class', xlab='Management procedure'
)

plot_whisker_mp_par(
  performances,
  y='status_mean', ylab="Status (average %B0)",
  x='class', xlab='Management procedure'
)

plot_whisker_mp_par(
  performances,
  y='safety_b20', ylab="Safety (probability > 20%B0)",
  x='class', xlab='Management procedure'
)

plot_whisker_mp_par(
  performances,
  y='cpue_mean_we_ps', ylab="Western purse seine CPUE",
  x='class', xlab='Management procedure'
) + geom_hline(yintercept=1,linetype=2)

plot_whisker_mp_par(
  performances,
  y='cpue_mean_ea_gn', ylab="Eastern gillnet CPUE",
  x='class', xlab='Management procedure'
) + geom_hline(yintercept=1,linetype=2)

plot_whisker_mp_par(
  performances,
  y='cpue_mean_ma_pl', ylab="Maldives pole-and-line CPUE",
  x='class', xlab='Management procedure'
) + geom_hline(yintercept=1,linetype=2)

plot_tradeoff(
  perfs,
  x = list(status='Status (average SB/SB0)'),
  y = list(yield='Yield (average annual catch, kt)'),
  colour = list(class='Class'),
  shape = list(class='Class'),
  bars=T
)
