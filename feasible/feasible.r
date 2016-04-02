library(plyr)
library(ggplot2)

sf <- read.table('../data/input/size_freqs.tsv',header=T)

# For each method calculate quantiles
temp <- ddply(sf,.(method,class),summarise,p=sum(proportion*size)/sum(size))
cumuls <- NULL
quantiles <- NULL
for(m in 0:3){
  ps <- subset(temp,method==m)$p
  if(length(ps)>0){
    cumul <- diffinv(ps)[-1]
    cumuls <- rbind(cumuls,data.frame(
      method = m,
      size = unique(sf$class),
      cumul = cumul
    ))
  }
}
cumuls <- within(cumuls,{
  length <- size*2+1
})

ggplot(cumuls,aes(x=length,y=cumul,colour=factor(method))) + 
  geom_line() + geom_hline(y=c(0.1,0.5,0.9)) + geom_vline(x=seq(20,80,10))

temp <- within(subset(cumuls,size %in% c(15,20,25,30,35)),{
  lower <- round(cumul*0.6,2)
  lower <- ifelse(lower<0,0,lower)
  upper <- round(cumul/0.6,2)
  upper <- ifelse(upper>1,1,upper)
})[,c('method','size','lower','upper')]

write.table(temp,"input/size_freqs_bounds.tsv",row.names=F,quote=F,sep='\t')

accepted <- read.table("output/accepted.tsv",header=T)
temp <- within(expand.grid(knot=0:6,method=0:3),{
  name <- paste0("selectivities.",method,".",knot,"..value")
})
ddply(melt(accepted[,temp$name]),.(variable),summarise,m=mean(value))


