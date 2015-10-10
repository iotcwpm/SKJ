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

		# Distributions of lengths of recruits
		recruits_lengths_mean = Uniform(5,1,10),
		recruits_lengths_cv = Uniform(0.15,0.1,0.2),

		# Weight at length
		weight_a = Fixed(5.32e-6),
		weight_b = Fixed(3.35),

		# Maturity at length
		maturity_inflection = Uniform(42,40,45),
		maturity_steepness = Uniform(5,4,6),

		# Natural mortality
		mortality_base = Uniform(0.8,0.5,0.9),
		mortality_exponent = Fixed(-0.29),

		# Length at age
		growth_rate = Fixed(0.3),	
		growth_assymptote = Fixed(75),
		growth_sd = Fixed(1),
		growth_cv = Fixed(0.2),

		# Movement
		movement_sw_nw = Uniform(0.05,0,0.4),
		movement_nw_ma = Uniform(0,0,0.4),
		movement_nw_ea = Uniform(0,0,0.4),
		movement_ma_ea = Uniform(0,0,0.4),
		movement_length_inflection = Uniform(50,0,100),
		movement_length_steepness = Uniform(10,1,10)
	)
)
