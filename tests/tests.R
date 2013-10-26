data = read.table("equilibrium_stable.txt",header=T)

with(data,plot(biomass_spawning_w,col=quarter))