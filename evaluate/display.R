source('../../common.R',local=T)

# Load in model outputs
load(c(
	'procedures',
	'samples',
	'performances',
	'track'
),from='../../evaluate/output')

# Calculate performance statistics from performance measures
perfs <- ddply(performances,.(procedure),summarise,
			   yield = mean(catches_total/1000*4),
			   yield_sd = sd(catches_total/1000*4),
			   stability = -mean(catches_mapc),
			   stability_sd = sd(catches_mapc),
			   status = mean(status_mean),
			   status_sd = sd(status_mean),
			   safety10 = 1-mean(status_b10),
			   safety10_sd = sd(status_b10),
			   safety20 = 1-mean(status_b20),
			   safety20_sd = sd(status_b20),
			   kobe_a = mean(kobe_a),
			   kobe_a_sd = sd(kobe_a),
			   kobe_d = mean(kobe_d),
			   kobe_d_sd = sd(kobe_d),
			   kobe_to_a = mean(kobe_to_a,na.rm=T),
			   kobe_to_a_sd = sd(kobe_to_a),
			   cpue_w_ps = mean(cpue_mean_w_ps),
			   cpue_w_ps_sd = sd(cpue_mean_w_ps),
			   cpue_m_pl = mean(cpue_mean_m_pl),
			   cpue_m_pl_sd = sd(cpue_mean_m_pl),
			   cpue_e_gn = mean(cpue_mean_e_gn),
			   cpue_e_gn_sd = sd(cpue_mean_e_gn)
)
# List of names of performance statistics
stats <- c(
	'yield','stability',
	'status','safety10','safety20',
	'kobe_a','kobe_d','kobe_to_a',
	'cpue_w_ps','cpue_m_pl','cpue_e_gn'
)
# Merge in procedures to facilitate summaries by procedure parameters
perfs <- merge(procedures,perfs)

# Augment track
track <- within(track,{
	replicate <- factor(replicate)
	procedure <- factor(procedure)
	biomass_spawners_overall = biomass_spawners_w + biomass_spawners_m + biomass_spawners_e
})

# Function for creating a signature for a MP instance
mp_sig <- function(proc){
	row <- subset(procedures,procedure==proc)
	pars <- row[,3:12][!is.na(row[,3:12])]
	paste0(row$class,'(',paste(pars,collapse=','),')')
}

# Pick some example replicates for use in traces based on
# average stock status in projections
temp <- subset(track,procedure==1 & year>=2015)
temp <- ddply(temp,.(replicate),summarise,by=mean(biomass_status))
temp <- temp$replicate[order(temp$by)]
replicate_egs <- temp[floor(length(temp)*c(0.2,0.5,0.8))]
replicate_egs <- factor(replicate_egs,levels=replicate_egs,ordered=T)

# Quantile plotting function
plot_ribbon <- function(data,y,ylab='Value',xlab='Year'){
	# Evaluation the y expression
	data$y <- eval(y,envir=data)
	# Calculate quantiles
	quantiles = ddply(data,.(year),function(data){
		quantiles = quantile(data$y,p=c(0.01,0.05,0.1,0.25,0.5,0.75,0.9,0.95,0.99),na.rm=T)
		data.frame(
			p1 = quantiles[1],
			p5 = quantiles[2],
			p10 = quantiles[3],
			p25 = quantiles[4],
			p50 = quantiles[5],
			p75 = quantiles[6],
			p90 = quantiles[7],
			p95 = quantiles[8],
			p99 = quantiles[9]
		)
	})
	# Plot
	temp <- subset(data,replicate %in% replicate_egs)
	temp$replicate <- factor(temp$replicate,levels=replicate_egs)
	p = ggplot() +
		geom_ribbon(data=quantiles,aes(x=year,ymin=p5,ymax=p95),alpha=0.1) + 
		geom_ribbon(data=quantiles,aes(x=year,ymin=p10,ymax=p90),alpha=0.1) + 
		geom_ribbon(data=quantiles,aes(x=year,ymin=p25,ymax=p75),alpha=0.1) +
		geom_line(data=temp,aes(x=year,y=y,colour=replicate),alpha=0.6) +
		geom_vline(x=2014.8,linetype=2) +
		geom_hline(y=0,alpha=0) +
		scale_colour_manual(values=hsv(c(0,0.66,0.33),0.6,0.6)) +
		labs(x=xlab,y=ylab) +
		theme(legend.position='none')
	print(p)
	p
}

# Plot of tradeoff between two performance statistics
plot_tradeoff <- function(data,x,y,colour,shape){
	data <- within(data,{
		xm = data[,names(x)]
		xsd = data[,paste0(names(x),'_sd')]
		ym = data[,names(y)]
		ysd = data[,paste0(names(y),'_sd')]
		colour = factor(data[,names(colour)])
		shape = factor(data[,names(shape)])
	})
	print(
		ggplot(data,aes(colour=colour,shape=shape)) +
			geom_point(aes(x=xm,y=ym),size=3,alpha=0.7) + 
			geom_segment(aes(x=xm-xsd,xend=xm+xsd,y=ym,yend=ym),alpha=0.5) +
			geom_segment(aes(x=xm,xend=xm,y=ym-ysd,yend=ym+ysd),alpha=0.5) +
			geom_vline(x=0,alpha=0) + 
			geom_hline(y=0,alpha=0) + 
			scale_shape_manual(values=1:10) + 
			labs(x=x[[1]],y=y[[1]],colour=colour[[1]],shape=shape[[1]])
	)
}
