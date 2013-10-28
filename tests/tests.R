data = read.table("recruiment_variation.txt",header=T)

with(data,plot(biomass_spawning_m,col=quarter+1))