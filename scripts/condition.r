# Presentation of feasibility conditioning results

# Read in files
load(c(
  'accepted',
  'rejected',
  'track'
),from='../../feasible/output')

# Add columns to accepted to allow rbinding
accepted$trial <- NA
accepted$time <- NA
accepted$year <- NA
accepted$quarter <- NA
accepted$criterion <- -1
# Bind together
all <- rbind(accepted,rejected)

# Get table of rejections
tabulate_rejects <- function() {
  round(table(rejected$criterion)/nrow(all)*100,2)
}

# Plot densities by rejection criterion
plot_densities <- function(param,label) {
  if (missing(label)) label <- param
  name <- paste0(param,".value")
  values <- within(rbind(
    data.frame(value=accepted[,name],criterion=-1),
    data.frame(value=rejected[,name],criterion=rejected$criterion)
  ),{
    criterion <- factor(criterion)
  })
  ggplot(values,aes(x=value)) + 
    geom_density(data=values,linetype=2,adjust=1/2) + # Prior
    geom_density(data=subset(values,criterion!=-1),aes(fill=criterion,colour=criterion),adjust=1/2,alpha=0.2) + # Rejection
    geom_density(data=subset(values,criterion==-1),linetype=1,adjust=1/2) + # Posterior
		labs(x="Value",y="Density",fill="Criterion",colour="Criterion")
}
