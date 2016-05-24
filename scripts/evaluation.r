# Presentation

# Read in output files
load(c(
	'procedures',
	'samples',
	'references',
	'performances',
	'track'
),from='../../evaluate/output')

performances <- within(performances,{
  catches_total <- catches_total/1000*4
  status_mean <- status_mean * 100
  safety_b20 <- (1-status_b20) *100
  stability_mapc <- -(catches_mapc * 100)
})

performances <- merge(procedures,performances)

# Calculate summary stats of performance statistics
perfs <- ddply(performances,.(procedure),summarise,
	yield = mean(catches_total),
	yield_sd = sd(catches_total),
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
	cpue_we_ps = mean(cpue_mean_we_ps),
	cpue_we_ps_sd = sd(cpue_mean_we_ps),
	cpue_ma_pl = mean(cpue_mean_ma_pl),
	cpue_ma_pl_sd = sd(cpue_mean_ma_pl),
	cpue_ea_gn = mean(cpue_mean_ea_gn),
	cpue_ea_gn_sd = sd(cpue_mean_ea_gn)
)

# List of names of performance statistics
stats <- c(
	'yield','stability',
	'status','safety10','safety20',
	'kobe_a','kobe_d','kobe_to_a',
	'cpue_we_ps','cpue_ma_pl','cpue_ea_gn'
)

# Merge in procedures to facilitate summaries by procedure parameters
perfs <- merge(procedures,perfs)

# Merge samples and performance
samples$replicate <- 1:nrow(samples)
sens <- merge(performances,samples)

# Augment track
track <- within(track,{
	replicate <- factor(replicate)
	procedure <- factor(procedure)
	biomass_spawners_overall <- biomass_spawners_we + biomass_spawners_ma + biomass_spawners_ea
})

# Function for creating a signature for a MP instance
mp_sig <- function(proc){
	row <- subset(procedures,procedure==proc)
	pars <- row[,3:12][!is.na(row[,3:12]) & nchar(row[,3:12])>0]
	paste0(row$class,'(',paste(pars,collapse=','),')')
}

# Pick some example replicates for use in traces based on
# average stock status in projections
temp <- subset(track,procedure==subset(procedures,class=="ConstEffort" & p1==100)$procedure & year>=2015)
temp <- ddply(temp,.(replicate),summarise,by=mean(biomass_status))
temp <- temp$replicate[order(temp$by)]
replicate_egs <- temp[floor(length(temp)*c(0.25,0.5,0.75))]
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
	  #geom_ribbon(data=quantiles,aes(x=year,ymin=p5,ymax=p95),alpha=0.1) +
		geom_ribbon(data=quantiles,aes(x=year,ymin=p10,ymax=p90),alpha=0.1) + 
		geom_ribbon(data=quantiles,aes(x=year,ymin=p25,ymax=p75),alpha=0.1) +
	  geom_line(data=quantiles,aes(x=year,y=p50),alpha=0.7) +
		geom_line(data=temp,aes(x=year,y=y,colour=replicate),alpha=0.5) +
		geom_vline(xintercept=2014.8,linetype=2,alpha=0.5) +
		geom_hline(yintercept=0,alpha=0) +
		scale_colour_manual(values=hsv(c(0,0.66,0.33),0.6,0.6)) +
		labs(x=xlab,y=ylab) +
		theme(legend.position='none')
	p
}

#' Function for plotting
plot_ribbon_catch_status <- function(proc,start=1950,title=TRUE){
  temp <- ddply(subset(track,procedure %in% c(-1,proc) & year>=start),.(year,replicate),summarise,catch=sum(catches_total/1e3))
  p1 <- plot_ribbon(temp,y=quote(catch),ylab='Catch (kt)') +
      theme(plot.title=element_text(size=rel(0.8)))
  if (title) p1 <- p1 + labs(title=mp_sig(proc))
  temp <- subset(track,quarter==3 & procedure %in% c(-1,proc) & year>=start)
  p2 <- plot_ribbon(temp,y=quote(biomass_status*100),ylab='Status (%B0)') + 
      geom_hline(yintercept=20,linetype=2,alpha=0.5) + 
      geom_hline(yintercept=40,linetype=2,alpha=0.5)
  multiplot(p1,p2)
}

# Whisker plot of a performance statistic versus an MP parameter
plot_whisker_mp_par <- function(data,y,ylab,yrefs,x,xlab,xrefs, show_mean=F){
  data$y <- data[,y]
  data$x <- data[,x]
  temp <- ddply(data,c('x'),function(data){
    quantiles = quantile(data$y,p=c(0.1,0.25,0.5,0.75,0.9),na.rm=T)
    data.frame(
      p10 = quantiles[1],
      p25 = quantiles[2],
      p50 = quantiles[3],
      p75 = quantiles[4],
      p90 = quantiles[5],
      mean = mean(data$y)
    )
  })
  plot <- ggplot(temp,aes(x=x))
  if (!missing(yrefs)) {
    plot <- plot + geom_hline(yintercept=yrefs,linetype=2,colour='grey')
  }
  if (!missing(xrefs)) {
    plot <- plot + geom_vline(xintercept=xrefs,linetype=2,colour='grey')
  }
  plot <- plot + 
    geom_point(aes(y=p50),size=4,colour='grey50') +
    geom_segment(aes(xend=x,y=p25,yend=p75),size=2,colour='grey50') + 
    geom_segment(aes(xend=x,y=p10,yend=p90),size=0.5,colour='grey50') +
    geom_hline(yintercept=0,alpha=0) + 
    labs(y=ylab,x=xlab)
  if (show_mean) plot <- plot + geom_point(aes(y=mean),size=4,shape=2)
  plot
}

plot_whisker_mp_par_multi <- function(data,x,xlab,xrefs) {
  multiplot(
    plot_whisker_mp_par(data, y='catches_total', ylab='Yield (Mean catch; kt)', yrefs=425, x=x, xlab=xlab, xrefs=xrefs),
    plot_whisker_mp_par(data, y='status_mean', ylab='Status (Mean %B0)', yrefs=c(20,40), x=x, xlab=xlab, xrefs=xrefs),
    plot_whisker_mp_par(data, y='safety_b20', ylab='Safety (Prop years B>20%B0)', x=x, xlab=xlab, xrefs=xrefs),
    plot_whisker_mp_par(data, y='stability_mapc', ylab='Stability (-MAPC, %)', x=x, xlab=xlab, xrefs=xrefs),
    cols=2
  )
}

stat_summary <- function(data,stat,label){
  x <- with(data,eval(parse(text=stat)))
  quantiles <- quantile(x,p=c(0.1,0.25,0.5,0.75,0.9),na.rm=T)
  data.frame(
    label = label,
    mean = mean(x),
    p10 = quantiles[1],
    p25 = quantiles[2],
    p50 = quantiles[3],
    p75 = quantiles[4],
    p90 = quantiles[5]
  )
}

table_stat_summary <- function(data) {
  temp <- rbind(
    stat_summary(data,'status_mean','Status (Mean %B0)'),
    stat_summary(data,'safety_b20','Safety (Prop. years B>20%B0)'),
    stat_summary(data,'catches_total','Yield (Mean catch; kt)'),
    stat_summary(data,'(1-catches_lower)*100','Yield (Years catch>=425kt %)'),
    stat_summary(data,'-stability_mapc','Stability (MAPC %)'),
    stat_summary(data,'control_downs*100','Stability (Years decrease %)'),
    stat_summary(data,'control_ups*100','Stability (Years increase %)')
  )
  row.names(temp) <- NULL
  temp
}
  
table_stat_summary_full <- function(data) {
  temp <- rbind(
    stat_summary(data,'status_mean','Status (Mean %B0)'),
    stat_summary(data,'f_ratio','Fishing intensity (F/F40%B0)'),
    stat_summary(data,'kobe_a*100','Kobe green (Years %)'),
    stat_summary(data,'kobe_b*100','Kobe top-right (Years %)'),
    stat_summary(data,'kobe_c*100','Kobe red (Years %)'),
    stat_summary(data,'kobe_d*100','Kobe bottom-left (Years %)'),
    stat_summary(data,'safety_b20','Safety (Prop. years B>20%B0)'),
    stat_summary(data,'catches_total','Yield (Mean catch; kt)'),
    stat_summary(data,'(1-catches_lower)*100','Yield (Years catch>=425kt %)'),
    stat_summary(data,'-stability_mapc','Stability (MAPC %)'),
    stat_summary(data,'catches_shut*100','Probability of shutdown (Years catch<1kt %)'),
    stat_summary(data,'control_downs*100','Stability (Years TAC decrease %)'),
    stat_summary(data,'control_ups*100','Stability (Years TAC increase %)'),
    stat_summary(data,'cpue_mean_we_ps','Western purse seine CPUE (relative to 2000-2015)'),
    stat_summary(data,'cpue_mean_ma_pl','Maldives pole and line CPUE (relative to 2000-2015)'),
    stat_summary(data,'cpue_mean_ea_gn','Eastern gill net CPUE (relative to 2000-2015)')
  )
  row.names(temp) <- NULL
  temp
}

# Plot of tradeoff between two performance statistics
plot_tradeoff <- function(data, x, y, colour, shape, xmin=NA, ymin=NA, bars=T, ref=NULL){
  data <- within(data,{
    xm = data[,names(x)]
    xsd = data[,paste0(names(x),'_sd')]
    ym = data[,names(y)]
    ysd = data[,paste0(names(y),'_sd')]
    colour = factor(data[,names(colour)])
    shape = factor(data[,names(shape)])
  })
  plot <- ggplot(data,aes(colour=colour,shape=shape)) +
    geom_point(aes(x=xm,y=ym),size=3,alpha=0.9) +
    scale_shape_manual(values=1:10) + 
    labs(x=x[[1]],y=y[[1]],colour=colour[[1]],shape=shape[[1]])
  if (!is.na(xmin)) {
    plot <- plot + xlim(xmin,NA)
  }
  if (!is.na(ymin)) {
    plot <- plot + ylim(ymin,NA)
  }
  if (bars) {
    plot <- plot +
      geom_segment(aes(x=xm-xsd,xend=xm+xsd,y=ym,yend=ym),alpha=0.5) +
      geom_segment(aes(x=xm,xend=xm,y=ym-ysd,yend=ym+ysd),alpha=0.5)
  }
  if (!is.null(ref)) {
    plot <- plot + geom_point(data=within(ref,{
      x <- ref[,names(x)]
      y <- ref[,names(y)]
    }), aes(x=x,y=y), shape=16, size=4, alpha=0.5)
  }
  plot
}

plot_tradeoff_multi <- function(data, x, y, first, second, bars=F, ref=NULL){
  multiplot(
    plot_tradeoff(data, x, y, first$colour, first$shape, bars=bars, ref=ref),
    plot_tradeoff(data, x, y, second$colour, second$shape, bars=bars, ref=ref)
  )
}
