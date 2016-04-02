source('common.R')
load("catches",from='parameters/output')

require(ggplot2)

ggplot(subset(catches,year<=2014),aes(x=year+quarter/4,y=value.1,colour=method)) + 
  geom_line() +
  facet_wrap(~region) + 
  labs(x='',y='Catch (t;quarter)',colour='Method')
