namespace IOSKJ {

/**
 * Model of the Indian Ocean skipjack tuna fishery. This class encapsulates both fish
 * population and fishing dynamics.
 */
class Model {
public:

	/**
	 * Fish numbers by region, age and size
	 */
	Grid<double,Region,Age,Size> numbers;

	/**
	 * Total biomass by region
	 */
	Grid<double,Region> biomass;
	
	/**
	 * @{
	 * @name Spawning
	 */

	/**
	 * The spawning fraction by quarter
	 */
	Grid<double,Quarter> spawning;
	

	/**
	 * The total spawning biomass by region
	 */
	Grid<double,Region> biomass_spawning;

	/**
	 * Total spawning biomass in each of the most recent quarters
	 * This is recorded in step() so that the biomass_spawning_unfished
	 * can be set by init()
	 */
	Grid<double,Quarter> biomass_spawning_overall;

	/**
	 * Unfished spawning biomass by quarter. It is necessary to have this by quarter
	 * because the proportion of mature fish that spawn varies by quarter.
	 */
	Grid<double,Quarter> biomass_spawning_unfished;

	/**
	 * @}
	 */

	/**
	 * @{
	 * @name Recruitment
	 */

	/**
	 * Unfished equlibrium recruitment (numbers)
	 */
	double recruits_unfished;

	/**
	 * Steepness of stock-recruit relation
	 */
	double recruits_steepness;

	/**
	 * Flag to turn on/off recruitment relation
	 * (if off then recruits_unfished is used)
	 */
	bool recruits_relation_on = true;

	/**
	 * Deterministic recruitment at time t 
	 */
	double recruits_determ;

	/**
	 * Standard deviation of recruitment vaiation
	 */
	double recruits_sd;

	/**
	 * Flag to turn on/off recruitment variation
	 */
	bool recruits_variation_on = true;

	/**
	 * Lognormal distibution to produces recruitment 
	 * deviations from recruit_realtion.
	 */
	Lognormal recruits_variation;

	/**
	 * Recruitment deviation (multiplier) at time t
	 */
	double recruits_deviation;

	/**
	 * Total number of recruits at time t
	 */
	double recruits;

	/**
	 * Proportion of recruits by region
	 */
	Grid<double,Region> recruits_regions;

	/**
	 * Proportion of recruits by size class
	 */
	double recruits_lengths_mean;
	double recruits_lengths_cv;
	Grid<double,Size> recruits_lengths;

	/**
	 * @}
	 */
	
	/**
	 * @{
	 * @name Length, weight, maturity
	 */
	
	/**
	 * Length at size s
	 */
	Grid<double,Size> lengths;
	const double lengths_step = 2;

	/**
	 * Scalar of length-weight relationship
	 */
	double weight_a;

	/**
	 * Exponent of length-weight relationship
	 */
	double weight_b;

	/**
	 * Weight at size s
	 */
	Grid<double,Size> weights;
	
	/**
	 * Inflection of maturity ogive
	 */
	double maturity_inflection;

	/**
	 * Steepness of maturiy ogive
	 */
	double maturity_steepness;

	/**
	 * Maturity at size s
	 */
	Grid<double,Size> maturities;

	/**
	 * @}
	 */
	
	/**
	 * @{
	 * @name Natural mortality
	 */
	
	/**
	 * Allometric exponent of the weight to natural mortality
	 * relationship
	 */
	double mortality_weight_exponent;
	
	/**
	 * Instantaneous rate of natural mortality at weight of 1kg
	 */
	double mortality;

	/**
	 * Maximum rate of natural mortality (exponential relationship
	 * can give very high natural mortality at small sizes)
	 */
	double mortality_max;

	/**
	 * Instantaneous rate of natural mortality for size s
	 */
	Grid<double,Size> mortality_rate;

	/**
	 * Quarterly rate of survival from natural mortality for size s
	 */
	Grid<double,Size> mortality_survival;

	/**
	 * @}
	 */

	/**
	 * @{
	 * @name Growth
	 */
	
	double growth_rate;
	double growth_assymptote;
	double growth_sd;
	double growth_cv;
	Grid<double,Size> growth_increments;
	Grid<double,SizeFrom,Size> growth;

	/**
	 * @}
	 */

	/**
	 * @{
	 * @name Movement
	 */
	
	Grid<double,RegionFrom,Region> movement_pars;
	Grid<double,RegionFrom,Region> movement;

	/**
	 * @}
	 */

	/**
	 * @{
	 * @name Selectivity and exploitation
	 */
	
	Grid<double,Method,SelectivityKnot> selectivity_points;

	Grid<double,Method,Size> selectivities;

	/**
	 * A switch used to turn on/off exploitation dynamics
	 * (e.g turn off for virgin equilibrium)
	 */
	bool exploitation_on = true;

	/**
	 * A switch used to turn on/off the calculation
	 * of exploitation rates from catches. Turned off to specify a particular
	 * exploitation rate when calculating MSY/Bmsy
	 */
	bool catches_on = true;

	/**
	 * The exploitation rate specified, for example, when calculating MSY/Bmsy
	 */
	double exploitation_rate_specified;

	/**
	 * Vulnerable biomass by region and method
	 */
	Grid<double,Region,Method> biomass_vulnerable;

	/**
	 * Catches by region and method
	 */
	Grid<double,Region,Method> catches;

	/**
	 * Catches by region and method given maximimum exploitation rate of
	 * one. This variable is useful for penalising against impossible dynamics.
	 */
	Grid<double,Region,Method> catches_taken;

	/**
	 * Exploitation rate by region and method for current time step
	 */
	Grid<double,Region,Method> exploitation_rate;

	/**
	 * Exploitation survival
	 */
	Grid<double,Region,Size> exploitation_survival;

	/**
	 * @}
	 */
	

	/**
	 * @{
	 * @name Tracking
	 *
	 * Tracking of various model variables during simulation.
	 * Mainly used in testing.
	 * Writing output is slow so this can be turned off using the TRACKING macro
	 */
	#if TRACKING

		bool track_on;
		std::ofstream track_file;

		void track_open(std::string filename){
			track_on = true;
			track_file.open(filename);
			track_file
				<<"year\t"
				<<"quarter\t"
				<<"recruits_determ\t"
				<<"recruits_deviation\t"
				<<"recruits\t"
				<<"biomass_spawning_overall\t"
				<<"biomass_spawning_w\t"
				<<"biomass_spawning_m\t"
				<<"biomass_spawning_e\t"
				<<"catches_w_ps\t"
				<<"catches_m_pl\t"
				<<"catches_e_gn\t"
				<<"exploitation_survival_m_20\t"
				<<"biomass_vulnerable_m_pl\t"
				<<"exp_rate_m_pl\t"
				<<std::endl;
		}

		void track(int year, int quarter){
			if(track_on){
				track_file
					<<year<<"\t"
					<<quarter<<"\t"
					<<recruits_determ<<"\t"
					<<recruits_deviation<<"\t"
					<<recruits<<"\t"
					<<biomass_spawning_overall(quarter)<<"\t"
					<<biomass_spawning(W)<<"\t"
					<<biomass_spawning(M)<<"\t"
					<<biomass_spawning(E)<<"\t"
					<<catches(W,PS)<<"\t"
					<<catches(M,PL)<<"\t"
					<<catches(E,GN)<<"\t"
					<<exploitation_survival(M,20)<<"\t"
					<<biomass_vulnerable(M,PL)<<"\t"
					<<exploitation_rate(M,PL)<<"\t"
					<<std::endl;
			}
		}

		void track_close(void){
			track_on = false;
			track_file.close();
		}

	#else

		void track_open(std::string filename){
		}

		void track(int year, int quarter){
		}

		void track_close(void){
		}

	#endif

	/**
	 * @}
	 */

	/**
	 * Set exploitation rate. Used in testing and in 
	 * equilibrium exploitation i.e. MSY/BMSY calculations
	 */
	void exploitation_rate_set(double value){
		exploitation_on = true;
		catches_on = false;
		exploitation_rate_specified = value;
	}

	/**
	 * @{
	 * @name Parameter setting methods
	 */

	/**
	 * Set recruitment distribution across regions to be uniform
	 */
	void recruits_uniform(void){
		recruits_regions = 1.0/recruits_regions.size();
	}

	/**
	 * Set movement parameters so that there is uniform movement.
	 *
	 * All elements set to 1/3. Mainly used for testing
	 */
	void movement_uniform(void){
		movement_pars = 1.0/movement_pars.size();
	}

	/**
	 * Set spawning seasonality parameters so that there is uniform
	 * spawning.
	 *
	 * All elements set to 1. Mainly used for testing.
	 */
	void spawning_uniform(void){
		spawning = 1.0;
	}

	//! @}

	/**
	 * Sample parameter values from prior probability distributions
	 */
	void sample(void){
	}

	/**
	 * Initialise various model variables based on current parameter values
	 */
	void initialise(void){
		// Initialise grids by size...	
		for(auto size : sizes){
			lengths(size) = 2*size.index()+1;
			weights(size) = weight_a * std::pow(lengths(size),weight_b);
			maturities(size) = 1/(1+std::pow(19,(maturity_inflection-lengths(size))/maturity_steepness));
			mortality_rate(size) = std::min(mortality * std::pow(weights(size),mortality_weight_exponent),mortality_max);
			mortality_survival(size) = std::exp(-0.25*mortality_rate(size));
		}

		// Initialise proportion of recruits by size
		Lognormal recruits_lengths_dist(recruits_lengths_mean,recruits_lengths_mean*recruits_lengths_cv);
		recruits_lengths = [&](Level<Size> size){
			double length = lengths(size);
			return recruits_lengths_dist.integrate(length-1,length+1);
		};

		// Initialise growth size transition matrix
		for(auto size : sizes){
			growth_increments(size) = (growth_assymptote-lengths(size))*(1-std::exp(-0.25*growth_rate));
		}
		for(auto size_from : size_froms){
			Level<Size> size_from_size(size_from);
			double length_from = lengths(size_from_size);
			double growth_increment = growth_increments(size_from_size);
			double mean = length_from + growth_increment;
			double sd = std::pow(std::pow(growth_sd,2)+std::pow(growth_increment*growth_cv,2),0.5);
			Normal distribution(mean,sd);
			for(auto size : sizes){
				double length_to = lengths(size);
				growth(size_from,size) = distribution.integrate(length_to-1,length_to+1);
			}
		}

		// Initialise movement matrix
		// Normalise so that movement proportions always sum to 1 for a particular region
		auto movement_sums = movement_pars(sum(),by(region_froms));
		for(auto region_from : region_froms){
			for(auto region : regions){
				movement(region_from,region) = 
					movement_pars(region_from,region)/movement_sums(region_from);
			}
		}

		// Initialise selectivity
		for(auto method : methods){
			// Extract values at knots
			Grid<double,SelectivityKnot> points;
			for(auto knot : selectivity_knots) points(knot) = selectivity_points(method,knot);
			// Create a spline
			Spline<double,double> selectivity_spline(
				{0,20,40,60,80},
				points
			);
			// Iterpolate with spline
			for(auto size : sizes){
				selectivities(method,size) = selectivity_spline.interpolate(lengths(size));
			}
		}

		// Normalise the recruits_region grid so that it sums to one
		recruits_regions /= sum(recruits_regions);

		// Initialise recruits_variation
		recruits_variation = Lognormal(1,recruits_sd);

		// During debug mode dump the model here for easy inspection
		// Done here before quilibrium() in case that fails
		#if DEBUG
			write();
		#endif

		/**
		 * The population is initialised to an unfished state
		 * by iterating with virgin recruitment until it reaches equibrium
		 * defined by less than 0.01% change in total biomass.
		 */
		// Turn off recruitment relationship, variation and exploitation
		recruits_relation_on = false;
		exploitation_on = false;
		// Reset the population, noting that with recruits_relation_on==false 
		// the population gets R0 each year
		numbers = 0.0;
		// Go to equilibrium
		equilibrium();
		// Turn on recruitment relationship etc again
		recruits_relation_on = true;
		exploitation_on = true;

		/**
		 * Once the population has converged to unfished equilibrium, the virgin
		 * spawning biomass in each quarter can be set.
		 */
		for(auto quarter : quarters){
			biomass_spawning_unfished(quarter) = biomass_spawning_overall(quarter);
		}

		// During debug mode dump the model here for easy inspection
		// Done after equilibrium() and biomass_spawning_unfished has been set
		#if DEBUG
			write();
		#endif
	}

	/**
	 * Perform a single time step
	 */
	void update(uint time){
		uint year = IOSKJ::year(time);
		uint quarter = IOSKJ::quarter(time);

		// Calculate total biomass and spawning biomass by region
		for(auto region : regions){
			double biomass_ = 0;
			double biomass_spawning_ = 0;
			for(auto age : ages){
				for(auto size : sizes){
					double biomass = numbers(region,age,size) * weights(size)/1000;
					biomass_ += biomass;
					biomass_spawning_ += biomass * maturities(size) * spawning(quarter);
				}
			}
			biomass(region) = biomass_;
			biomass_spawning(region) = biomass_spawning_;

		} 
		biomass_spawning_overall(quarter) = sum(biomass_spawning);

		// Recruits
		if(recruits_relation_on){
			// Stock-recruitment realtion is active so calculate recruits based on 
			// the spawning biomass in the previous time step
			//! @todo check this equation
			recruits_determ = 
				4 * recruits_steepness * recruits_unfished * biomass_spawning_unfished(quarter) / (
					(5*recruits_steepness-1)*biomass_spawning_overall(quarter) + 
					biomass_spawning_unfished(quarter)*(1-recruits_steepness)
				);
		} else {
			// Stock-recruitment relation is not active so recruitment is just r0.
			recruits_determ = recruits_unfished;
		}
		
		if(recruits_variation_on){
			recruits_deviation = recruits_variation.random();
		}

		recruits = recruits_determ * recruits_deviation;

		// Ageing and recruitment
		for(auto region : regions){
			for(auto size : sizes){
				// Oldest age class accumulates 
				numbers(region,ages.size()-1,size) += numbers(region,ages.size()-2,size);

				// For most ages just "shuffle" along
				for(uint age=ages.size()-2;age>0;age--){
					numbers(region,age,size) = numbers(region,age-1,size);
				}

				// Recruits are evenly distributed over regions and over sizes
				// according to `initials`
				numbers(region,0,size) = recruits * recruits_regions(region) * recruits_lengths(size);
			}
		}

		// Exploitation rate
		if(exploitation_on){
			if(catches_on){
				for(auto region : regions){
					for(auto method : methods){
						// Calculate vulnerable biomass in this region
						double biomass_vuln = 0;
						for(auto age : ages){
							for(auto size : sizes){
								biomass_vuln += numbers(region,age,size) * weights(size)/1000 * selectivities(method,size);
							}
						}
						// Calculate exploitation rate
						double er = 0;
						if(biomass_vuln>0) er = catches(region,method)/biomass_vuln;
						if(er>1) er = 1;
						// Assign to variables
						biomass_vulnerable(region,method) = biomass_vuln;
						exploitation_rate(region,method) = er;
						catches_taken(region,method) = er * biomass_vuln;
					}
				}
			} else {
				exploitation_rate = exploitation_rate_specified;
			}
			// Pre-calculate the exploitation_survival for each region and size
			for(auto region : regions){
				for(auto size : sizes){
					double proportion_taken = 0;
					for(auto method : methods){
						proportion_taken += exploitation_rate(region,method) * selectivities(method,size);
					}
					exploitation_survival(region,size) = 1-proportion_taken;
				}
			}
		}
	
		// Mortality, growth and movement
		auto numbers_temp = numbers;
		for(auto region : regions){
			for(auto age : ages){
				for(auto size : sizes){
					double number = 0;
					for(auto region_from : region_froms){
						for(auto size_from : size_froms){ 
							Level<Region> rf(region_from);
							Level<Size> sf(size_from);
							number += 	numbers(rf,age,sf) * 
										growth(size_from,size) * 
										mortality_survival(sf) * 
										(exploitation_on?exploitation_survival(rf,sf):1) * 
										movement(region_from,region);
						}
					}
					numbers_temp(region,age,size) = number;
				}
			}
		}
		numbers = numbers_temp;

		// Tracking
		track(year,quarter);
	}

	/**
	 * Move the population to a deterministic equilibrium by iterating over
	 * time until biomass in each region remains stable
	 */
	void equilibrium(void){
		// Turn off recruitment variation
		recruits_variation_on = false;
		// Iterate until there is a very minor change in biomass
		uint steps = 0;
		const uint steps_max = 1000;
		Grid<double,Region> biomass_prev = 1;
		while(steps<steps_max){
			// It is necessary to update the model for each quarter so that quarterly differences
			// in dynamics (e.g. spawning proportion) are incorporated
			for(uint quarter=0;quarter<4;quarter++) update(quarter);

			double diffs = 0;
			for(auto region : regions){
				diffs += fabs(biomass(region)-biomass_prev(region))/biomass_prev(region);
			}
			if(diffs<0.0001) break;
			biomass_prev = biomass;

			#if DEBUG
				std::cout<<steps<<"\t"<<biomass(W)<<"\t"<<biomass(M)<<"\t"<<biomass(E)<<"\t"<<diffs<<std::endl;
			#endif

			steps++;
		}
		// Throw an error if there was no convergence
		assert(steps<steps_max);
		// Turn on recruitment deviation again
		recruits_variation_on = true;
	}

	/**
	 * Write model attributes to files for examination
	 */
	void write(void){
		numbers.write("output/numbers.tsv");
		lengths.write("output/lengths.tsv");
		weights.write("output/weights.tsv");
		maturities.write("output/maturities.tsv");
		mortality_rate.write("output/mortality-rate.tsv");
		spawning.write("output/spawning.tsv");
		biomass_spawning_unfished.write("output/biomass-spawning-unfished.tsv");
		growth_increments.write("output/growth-increments.tsv");
		growth.write("output/growth.tsv");
		movement.write("output/movement.tsv");
		selectivities.write("output/selectivities.tsv");
	}
};

}
