source('ioskj.r')

run(
	list(
		# Stock recruitment relation and variability
		spawners_unfished = Uniform(1.8e6,1e6,5e6),
		recruits_steepness = Uniform(0.8,0.7,1),
		recruits_sd = Uniform(0.6,0.4,0.8),

		# Quarterly spawning fractions
		spawning_0 = Uniform(0.8,0.7,1),
		spawning_1 = Uniform(0.5,0.3,0.7),
		spawning_2 = Uniform(0.8,0.7,1),
		spawning_3 = Uniform(0.5,0.3,0.7),

		# Growth
		growth_rate_1 = Uniform(1.12,0,0),
		growth_rate_2 = Uniform(0.33,0,0),
		growth_assymptote = Uniform(71.6,0,0),
		growth_stanza_inflection = Uniform(0.95,0,0),
		growth_stanza_steepness = Uniform(24.8,0,0),
		growth_cv_0 = Uniform(0.1,0,0),
		growth_cv_old = Uniform(0.1,0,0),

		# Weight at length
		weight_a = Fixed(5.32e-6),
		weight_b = Fixed(3.35),

		# Maturity at length
		maturity_inflection = Uniform(42,40,45),
		maturity_steepness = Uniform(5,4,6),

		# Natural mortality
		mortality_base = Uniform(0.8,0.5,0.9),
		mortality_exponent = Fixed(-0.29),

		# Movement
		movement_sw_nw = Uniform(0.05,0,0.4),
		movement_nw_ma = Uniform(0,0,0.4),
		movement_nw_ea = Uniform(0,0,0.4),
		movement_ma_ea = Uniform(0,0,0.4),
		movement_length_inflection = Uniform(50,0,100),
		movement_length_steepness = Uniform(10,1,10)
	)
)
