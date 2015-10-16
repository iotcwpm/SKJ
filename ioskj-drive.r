#!/usr/bin/env Rscript

source('ioskj.r')

# Definition of parameter default values and priors
# Possibilities are:
#   Fixed(value)
#   Uniform(default,lower,upper)
#   Normal(default,mean,sd)
#   Beta(default,alpha,beta)
# It is not possible to change the distribution type here
pars <- list(
	# Stock recruitment relation and variability
	spawners_unfished = Uniform(2e6,1e6,6e6),
	recruits_steepness = Beta(0.9,10,2),
	recruits_sd = Uniform(0.6,0.3,0.8),

	# Quarterly spawning fractions
	spawning_0 = Uniform(0.8,0.7,1),
	spawning_1 = Uniform(0.5,0.3,0.7),
	spawning_2 = Uniform(0.8,0.7,1),
	spawning_3 = Uniform(0.5,0.3,0.7),

	# Proportion of recruits by region
	# (relative to NW region)
	recruits_sw = Uniform(0.2,0.01,0.5),
	recruits_ma = Uniform(0.2,0.01,0.5),
	recruits_ea = Uniform(0.2,0.01,0.5),

	# Weight at length
	weight_a = Fixed(5.32e-6),
	weight_b = Fixed(3.35),

	# Maturity at length
	maturity_inflection = TruncatedNormal(40,40,2,30,50),
	maturity_steepness = TruncatedNormal(5,5,0.5,0.1,20),

	# Natural mortality
	mortality_base = Uniform(0.8,0.3,1.2),
	mortality_exponent = TruncatedNormal(-0.29,-0.29,0.07,-1,0),

	# Growth
	growth_rate_1 = TruncatedNormal(1.12,1.12,0.112,0.1,2),
	growth_rate_2 = TruncatedNormal(0.33,0.33,0.033,0.1,2),
	growth_assymptote = TruncatedNormal(71.6,71.6,7.16,50,100),
	growth_stanza_inflection = TruncatedNormal(0.95,0.95,0.095,0.1,10),
	growth_stanza_steepness = TruncatedNormal(24.8,24.8,2.48,0.1,100),
	growth_age_0 = Fixed(-0.3),
	growth_cv_0 = Uniform(0.1,0.01,0.3),
	growth_cv_old = Uniform(0.1,0.01,0.3),

	# Movement
	movement_sw_nw = Uniform(0.05,0,0.4),
	movement_nw_ma = Uniform(0.05,0,0.4),
	movement_nw_ea = Uniform(0.05,0,0.4),
	movement_ma_ea = Uniform(0.05,0,0.4),
	movement_length_inflection = Uniform(50,1,100),
	movement_length_steepness = Uniform(10,1,30)
)

# Do a single run 1950-2039 with parameter
# default values and default "procedure" of
# constant current catches by region.
# See outputs in `model/output`, `data/output` etc
run(
	pars = pars
)
