#' Script for plotting and summarising data fits

source('common.R')

load(c(
  'm_pl_cpue','w_ps_cpue','size_freqs','z_ests'
))

m_pl_cpue = subset(m_pl_cpue,data_year>2000)

# Maldive PL CPUE
ggplot(m_pl_cpue,aes(x=data_year+quarter/4)) + 
  geom_line(aes(y=expected)) + 
  geom_point(aes(y=observed,colour=factor(quarter),shape=factor(quarter)),size=3) + 
  geom_hline(y=0,alpha=0) +
  scale_shape_manual(values=1:4) +
  labs(x='Year',y='Maldive pole and line CPUE index',colour='Quarter',shape='Quarter')

# West PS CPUE
ggplot(w_ps_cpue,aes(x=data_year)) + 
  geom_line(aes(y=expected)) + 
  geom_point(aes(y=observed),shape=1,size=3) + 
  geom_hline(y=0,alpha=0) +
  labs(x='Year',y='West purse siene CPUE index')

# Size frequencies by region, method, quarter
size_freqs_over_years = ddply(size_freqs,.(region,method,quarter,length),summarise,
  observed=mean(observed,na.rm=T),
  expected=mean(expected,na.rm=T)
)
ggplot(size_freqs_over_years,aes(x=length,colour=factor(quarter))) + 
  geom_vline(x=seq(0,80,20),linetype=3,colour='grey') +
  geom_line(aes(y=expected)) + 
  geom_point(aes(y=observed),shape=1,size=3) + 
  geom_hline(y=0,alpha=0) + 
  facet_grid(region~method) +
  labs(x='Length (cm)',y='Proportion',colour='Quarter')

# Summary statistics of size frequencies
size_freqs_summ = ddply(size_freqs,.(region,method,data_year,quarter),summarise,
  observed=sum(length*observed,na.rm=T),
  expected=sum(length*expected,na.rm=T)
)
size_freqs_summ = within(size_freqs_summ,{
  observed[observed==0] = NA
})
ggplot(size_freqs_summ,aes(x=data_year+quarter/4,colour=factor(quarter))) + 
  geom_line(aes(y=expected)) + 
  geom_point(aes(y=observed),shape=1,size=3) + 
  facet_grid(region~method) +
  labs(x='Year',y='Mean length (cm)',colour='Quarter')


# Size freqs by year and quarter for a particular region & method
size_freqs_sub <- function(region_,method_){
  ggplot(subset(size_freqs,region==region_&method==method_),aes(x=length)) + 
    geom_line(aes(y=expected)) + 
    geom_point(aes(y=observed),shape=1,size=3) + 
    geom_hline(y=0,alpha=0) + 
    facet_grid(data_year~quarter)
}
size_freqs_sub('W','PS')
size_freqs_sub('W','GN')
size_freqs_sub('M','PL')
size_freqs_sub('M','OT')

size_freqs_by_year = ddply(size_freqs,.(region,method,data_year,length),summarise,
  observed=mean(observed,na.rm=T),
  expected=mean(expected,na.rm=T)
)
ggplot(subset(size_freqs_by_year,region=='M'&method=='PL'),aes(x=length)) + 
  geom_line(aes(y=expected)) + 
  geom_point(aes(y=observed),shape=1,size=2) + 
  geom_hline(y=0,alpha=0) + 
  facet_wrap(~data_year)

ggplot(subset(size_freqs,region=='M'&method=='PL'&uncertainty>1000),aes(x=length)) + 
  geom_line(aes(y=expected)) + 
  geom_point(aes(y=observed,colour=factor(quarter)),shape=1,size=2) + 
  geom_hline(y=0,alpha=0) + 
  facet_wrap(~data_year) +
  labs(x="Length (cm)",y="Proportion",colour='Quarter')


ggplot(subset(size_freqs,region=='W'&method=='PS'&uncertainty>1000),aes(x=length)) + 
  geom_line(aes(y=expected)) + 
  geom_point(aes(y=observed,colour=factor(quarter)),shape=1,size=2) + 
  geom_hline(y=0,alpha=0) + 
  facet_wrap(~data_year) +
  labs(x="Length (cm)",y="Proportion",colour='Quarter')


z_ests = subset(z_ests,is.finite(expected))
ggplot(z_ests,aes(x=data_year+quarter/4)) + 
  geom_line(aes(y=expected)) + 
  geom_point(aes(y=observed,colour=factor(quarter)),shape=1,size=3) + 
  geom_errorbar(aes(ymin=observed-uncertainty,ymax=observed+uncertainty,colour=factor(quarter)),width=0) +
  geom_hline(y=0,alpha=0) +
  facet_grid(z_size~.) +
  labs(x='Year',y='Instantaneous rate of total mortality (Z)',colour='Quarter')
