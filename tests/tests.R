data = read.table("recruiment_variation.txt",header=T)

with(data,{
  par(mfrow=c(3,1),mar=c(0,0,0,0))
  plot(biomass_spawning_w,col=quarter+1))
  plot(biomass_spawning_m,col=quarter+1))
  plot(biomass_spawning_e,col=quarter+1))
})
