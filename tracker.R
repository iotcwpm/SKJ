source('common.R')

track = read.table("output/track.tsv",header=T)

ggplot(track,aes(x=year+quarter/4)) + 
  geom_line(aes(y=recruits_determ))

ggplot(track,aes(x=year+quarter/4)) + 
  geom_line(aes(y=recruits_deviation))


ggplot(track,aes(x=year+quarter/4)) + 
  geom_line(aes(y=biomass_spawning_overall))

ggplot(track,aes(x=year+quarter/4)) + 
  geom_line(aes(y=biomass_spawning_w,colour='W')) + 
  geom_line(aes(y=biomass_spawning_m,colour='M')) + 
  geom_line(aes(y=biomass_spawning_e,colour='E'))


ggplot(track,aes(x=year+quarter/4)) + geom_line(aes(y=catches_w_ps))
ggplot(track,aes(x=year+quarter/4)) + geom_line(aes(y=catches_m_pl))
ggplot(track,aes(x=year+quarter/4)) + geom_line(aes(y=catches_e_gn))

ggplot(track,aes(x=year+quarter/4)) + geom_line(aes(y=biomass_vulnerable_m_pl))
ggplot(track,aes(x=year+quarter/4)) + geom_line(aes(y=exp_rate_m_pl))