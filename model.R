source('common.R')

load(c(
  'lengths','recruits_sizes',
  'growth_increments','growth',
  'mortalities',
  'weights','maturities','selectivities',
  'movement',
  'numbers'
))

# Size of recruits
ggplot(recruits_sizes,aes(x=length,y=value)) +
  geom_line() +
  labs(x="Length (cm)",y="Proportion")

# Growth increments
ggplot(growth_increments,aes(x=length,y=value)) +
  geom_hline(y=0,colour='grey') +
  geom_line() +
  labs(x="Length (cm)",y="Quarterly growth increment (cm)")

# Growth transition matrix
ggplot(growth) +
  geom_tile(aes(y=length,x=length_from,fill=value)) +
  scale_fill_gradient(low="white",high="red") + 
  geom_abline(a=0,b=1) + 
  labs(x="Length from (cm)",y="Length to (cm)",fill="Proportion")

# Mortalities
ggplot(mortalities,aes(x=length,y=value)) +
  geom_hline(y=0,colour='grey') +
  geom_line() +
  labs(x="Length (cm)",y="Instantaneous annual rate or natural mortality")

# Weights
ggplot(weights,aes(x=length,y=value)) +
  geom_line() +
  labs(x="Length (cm)",y="Weight (kg)")

# Maturities
ggplot(maturities,aes(x=length,y=value)) +
  geom_line() +
  labs(x="Length (cm)",y="Proportion mature")

# Movement transition matrix
ggplot(movement,aes(y=region_from,x=region,fill=value,label=round(value,4))) +
  geom_tile() +
  scale_fill_gradient(low="white",high="red") + 
  geom_text() +
  labs(y="Region from",x="Region to",fill="Proportion")

# Equilbrium numbers at age and size and region
ggplot(numbers) + 
  geom_line(aes(x=length,y=value/1e6)) +
  facet_grid(age~region,scale='free_y') + 
  labs(x="Length (cm)",y="Number (mil.)")

# Equilibrium numbers at size by region
ggplot(ddply(numbers,.(region,length),summarise,numbers=sum(value)/1e6)) +
  geom_line(aes(x=length,y=numbers)) + 
  geom_point(aes(x=length,y=numbers)) +
  facet_grid(.~region) + 
  labs(x="Length (cm)",y="Number (mil.)")

# Selectivities
ggplot(selectivities,aes(x=length,y=value,colour=method))+
  geom_line() + 
  labs(x="Length (cm)",y="Selectivity (prop.)",colour='Method')




