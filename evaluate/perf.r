# Examine the robustness of alternative
# measures of central tendency for a lognormally distributed
# variable
library(ggplot2)

geomean<-function(x)exp(mean(log(x)))

summary <- NULL
for(cv in seq(0.1,0.9,0.1)){
  samples <- NULL
  for(i in 1:1000){
    # Simulate 25 years of a lognormally distributed
    # variable with a mean of 0.2
    x <- rlnorm(25,log(0.2),cv)
    # Calculate statistics
    samples <- rbind(samples,data.frame(
      md = median(x),
      gm = geomean(x),
      am = mean(x)
    ))
  }
  # Calculate standard deviation of measures
  # More robust measures will have lower sd over the samples
  summary <- rbind(summary,data.frame(
    cv = cv,
    statistic = c('md','gm','am'),
    me = c(mean(samples$md),mean(samples$gm),mean(samples$gm)),
    sd = c(sd(samples$md),sd(samples$gm),sd(samples$gm))
  ))
}

ggplot(summary,aes(x=cv,y=sd,colour=statistic,shape=statistic)) + 
  geom_point(size=3) + geom_line()

ggplot(summary,aes(x=cv,y=me,ymin=me-sd,ymax=me+sd,colour=statistic)) + 
  geom_point() + geom_errorbar(width=0.05)
