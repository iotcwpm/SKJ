#pragma once

#include "dimensions.hpp"
#include "distributions.hpp"

namespace IOSKJ {

/**
 * Model of the Indian Ocean skipjack tuna fishery. This class encapsulates the dynamics
 * of both the fish population and fishing.
 */
class Model {
public:

	/**
	 * Fish numbers by region, age and size
	 */
	Array<double,Region,Age,Size> numbers;

	/**
	 * Total biomass by region
	 */
	Array<double,Region> biomass;
	
	/**
	 * @{
	 * @name Spawning
	 */

	/**
	 * The spawning fraction by quarter
	 */
	Array<double,Quarter> spawning;
	

	/**
	 * The total spawning biomass by region
	 */
	Array<double,Region> biomass_spawning;

	/**
	 * Total spawning biomass in each of the most recent quarters
	 * This is recorded in `update()` so that the biomass_spawning_unfished
	 * can be set by `initialise()`
	 */
	Array<double,Quarter> biomass_spawning_overall;

	/**
	 * Unfished spawning biomass by quarter. It is necessary to have this by quarter
	 * because the proportion of mature fish that spawn varies by quarter.
	 */
	Array<double,Quarter> biomass_spawning_unfished;

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
	Array<double,Region> recruits_regions;

	/**
	 * Proportion of recruits by size class
	 */
	double recruits_lengths_mean;
	double recruits_lengths_cv;
	Array<double,Size> recruits_sizes;

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
	Array<double,Size> lengths;
	const double lengths_step = 2;

	/**
	 * Weight at length power funciton
	 */
	double weight_length_a;
	double weight_length_b;

	/**
	 * Weight at size
	 */
	Array<double,Size> weights;
	
	/**
	 * Maturity at length logistic function
	 */
	double maturity_length_inflection;
	double maturity_length_steepness;

	/**
	 * Maturity at size
	 */
	Array<double,Size> maturities;

	/**
	 * @}
	 */
	
	/**
	 * @{
	 * @name Natural mortality
	 */
	
	/**
	 * Instantaneous rate of natural mortality at weight of 1kg
	 */
	double mortality_base;
	
	/**
	 * Allometric exponent of the weight to natural mortality
	 * relationship
	 */
	double mortality_exponent;

	/**
	 * Size with maximum rate of natural mortality (exponential relationship
	 * can give very high natural mortality at small sizes). Note this is not a 
	 * length but a 0-based size class
	 */
	Level<Size> mortality_cap_size = 10;

	/**
	 * Instantaneous rate of natural mortality for size s
	 */
	Array<double,Size> mortalities;

	/**
	 * Quarterly rate of survival from natural mortality for size s
	 */
	Array<double,Size> mortalities_survival;

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
	Array<double,Size> growth_increments;
	Array<double,SizeFrom,Size> growth;

	/**
	 * @}
	 */

	/**
	 * @{
	 * @name Movement
	 */
	
	Array<double,RegionFrom,Region> movement_pars;
	Array<double,RegionFrom,Region> movement;

	/**
	 * @}
	 */

	/**
	 * @{
	 * @name Selectivity and exploitation
	 */
	
	/**
	 * Lengths at each selectivity knot
	 */
	Array<double,SelectivityKnot> selectivity_lengths = {20,30,40,50,60,70,80};

	/**
	 * Proportion selected at each selectivity knot for each method
	 */
	Array<double,Method,SelectivityKnot> selectivity_values;

	/**
	 * Selectivities by method and size
	 */
	Array<double,Method,Size> selectivities;

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
	Array<double,Region,Method> biomass_vulnerable;

	/**
	 * Catches by region and method
	 */
	Array<double,Region,Method> catches;

	/**
	 * Catches by region and method given maximimum exploitation rate of
	 * one. This variable is useful for penalising against impossible dynamics.
	 */
	Array<double,Region,Method> catches_taken;

	/**
	 * Exploitation rate by region and method for current time step
	 */
	Array<double,Region,Method> exploitation_rate;

	/**
	 * Exploitation survival
	 */
	Array<double,Region,Size> exploitation_survival;

	/**
	 * @}
	 */
	

	/**
	 * @name Data related "nuisance" parameters
	 * 
	 * @{
	 */
	
	/**
	 * Lengths at each selectivity knot
	 *
	 * The default values below are simply the means of the observed 
	 * standardised CPUE index by quarter.
	 */
	Array<double,Quarter> m_pl_quarter = {0.97,0.87,0.97,1.19};

	/**
	 * @}
	 */


	/**
	 * @name Variable getting methods
	 * 
	 * @{
	 */
	
	/**
	 * Get the stock status (spawning biomass as a fraction of pristine)
	 */
	double biomass_status(uint time){
		uint quarter = IOSKJ::quarter(time);
		return biomass_spawning_overall(quarter)/biomass_spawning_unfished(quarter);
	}

	//! @}

	/**
	 * @name Parameter setting methods
	 * 
	 * @{
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
	 * @name Dynamics setting methods
	 * 
	 * @{
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
	 * Set instantaneous rate of fishing mortality (F).
	 * Like `exploitation_rate_set` but uses F insted of exp. rate.
	 */
	void fishing_mortality_set(double value){
		exploitation_rate_set(1-std::exp(-value));
	}

	//! @}

	/**
	 * Initialise various model variables based on current parameter values
	 */
	void initialise(void){
		// Initialise grids by size...	
		for(auto size : sizes){
			double length = 2*size.index()+1;
			lengths(size) = length;
			weights(size) = 1;//weight_length_a*std::pow(length,weight_length_b);
			maturities(size) = 1.0/(1.0+std::pow(19,(maturity_length_inflection-length)/maturity_length_steepness));
		}

		// Mortality at size
		// The maximum mortality is determined by the mortality_cap which defines
		// the _size_ at which mortality is highest. Because of this, mortalities needs
		// to be initalised after weights have been initialised
		for(auto size : sizes){
			double mortality_weight = (size<mortality_cap_size)?weights(mortality_cap_size):weights(size);
			mortalities(size) = mortality_base * std::pow(mortality_weight,mortality_exponent);
			mortalities_survival(size) = std::exp(-0.25*mortalities(size));
		}

		// Initialise proportion of recruits by size
		Normal recruits_lengths_dist(
			recruits_lengths_mean,
			recruits_lengths_mean*recruits_lengths_cv
		);
		for(auto size : sizes){
			double length = lengths(size);
			recruits_sizes(size) = recruits_lengths_dist.integral(length-1,length+1);
		};

		// Initialise growth size transition matrix
		for(auto size : sizes){
			double increment = (growth_assymptote-lengths(size))*(1-std::exp(-0.25*growth_rate));
			growth_increments(size) = (increment>0)?increment:0;
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
				growth(size_from,size) = distribution.integral(length_to-1,length_to+1);
			}
		}

		// Initialise movement matrix
		// Normalise so that movement proportions always sum to 1 for a particular region
		auto movement_sums = movement_pars(sum,by(region_froms));
		for(auto region_from : region_froms){
			for(auto region : regions){
				movement(region_from,region) = 
					movement_pars(region_from,region)/movement_sums(region_from);
			}
		}

		// Initialise selectivity
		for(auto method : methods){
			// Iterpolate with piecewise spline
			// Ensure that selectivity is between 0 and 1 since spline
			// can produce values outside this range even if knots are not
			double max = 0;
			for(auto size : sizes){
				double length = lengths(size);
				double selectivity;
				if(length<selectivity_lengths(0)) selectivity = 0;
				else{
					for(uint knot=0;knot<selectivity_knots.size()-1;knot++){
						if(selectivity_lengths(knot)<=length and length<selectivity_lengths(knot+1)){
							selectivity = selectivity_values(method,knot) + (length-selectivity_lengths(knot)) * (
								selectivity_values(method,knot+1)-selectivity_values(method,knot))/(
								selectivity_lengths(knot+1)-selectivity_lengths(knot)
							);
						}
					}
				}
				if(selectivity<0) selectivity = 0;
				else if(selectivity>max) max = selectivity;
				selectivities(method,size) = selectivity;
			}
			for(auto size : sizes) selectivities(method,size) /= max;
		}

		// Normalise the recruits_region grid so that it sums to one
		recruits_regions /= recruits_regions(sum);

		// Initialise recruits_variation
		recruits_variation = Lognormal(1,recruits_sd);

		// During debug mode dump the model here for easy inspection
		// Done here before equilibrium() in case that fails
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
		biomass_spawning_overall(quarter) = biomass_spawning(sum);

		// Recruits
		if(recruits_relation_on){
			// Stock-recruitment relation is active so calculate recruits based on 
			// the spawning biomass in the previous time step
			double h = recruits_steepness;
			double r0 = recruits_unfished;
			double s0 = biomass_spawning_unfished(quarter);
			double s = biomass_spawning_overall(quarter);
			recruits_determ =  4*h*r0*s/((5*h-1)*s+s0*(1-h));
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
				numbers(region,0,size) = recruits * recruits_regions(region) * recruits_sizes(size);
			}
		}

		// Exploitation rate
		if(exploitation_on){
			// Calculate vulnerable biomass
			for(auto region : regions){
				for(auto method : methods){
					double biomass_vuln = 0;
					for(auto age : ages){
						for(auto size : sizes){
							biomass_vuln += numbers(region,age,size) * weights(size)/1000 * 
								selectivities(method,size);
						}
					}
					biomass_vulnerable(region,method) = biomass_vuln;
					double er = 0;
					if(catches_on){
						// Calculate exploitation_rate from catches and biomass_vulnerable
						if(biomass_vuln>0) er = catches(region,method)/biomass_vuln;
						if(er>1) er = 1;
					} else {
						// Use specified exp rate
						er = exploitation_rate_specified;
					}
					exploitation_rate(region,method) = er;
					catches_taken(region,method) = er * biomass_vuln;
				}
			}
			// Pre-calculate the exploitation_survival for each region and size
			for(auto region : regions){
				for(auto size : sizes){
					double proportion_taken = 0;
					for(auto method : methods){
						proportion_taken += exploitation_rate(region,method) * selectivities(method,size);
					}
					exploitation_survival(region,size) = (proportion_taken<1)?(1-proportion_taken):0;
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
										mortalities_survival(sf) * 
										(exploitation_on?exploitation_survival(rf,sf):1) * 
										movement(region_from,region);
						}
					}
					numbers_temp(region,age,size) = number;
				}
			}
		}
		numbers = numbers_temp;
	}

	/**
	 * Move the population to a deterministic equilibrium by iterating over
	 * time until biomass in each region remains stable
	 */
	void equilibrium(void){
		#if DEBUG
			std::cout<<"************equilibrium()**************\n";
		#endif
		// Turn off recruitment variation (saving current setting
		// for putting back at the end of this method)
		bool recruits_variation_on_setting = recruits_variation_on;
		recruits_variation_on = false;
		// Iterate until there is a very minor change in biomass
		uint steps = 0;
		const uint steps_max = 1000;
		Array<double,Region> biomass_prev = 1;
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

			// Throw an error if undefined biomass
			if(not std::isfinite(biomass(W)+biomass(M)+biomass(E))) throw std::runtime_error("Biomass is not finite. Check inputs");

			steps++;
		}
		// Throw an error if there was no convergence
		if(steps>steps_max) throw std::runtime_error("No convergence in equilibrium() function");
		// Turn on recruitment deviation again
		recruits_variation_on = recruits_variation_on_setting;
	}

	/**
	 * Write model attributes to files for examination
	 */
	void write(void){
		lengths.write("model/output/lengths.tsv");
		recruits_sizes.write("model/output/recruits_sizes.tsv");

		growth_increments.write("model/output/growth_increments.tsv");
		growth.write("model/output/growth.tsv");

		mortalities.write("model/output/mortalities.tsv");

		weights.write("model/output/weights.tsv");
		maturities.write("model/output/maturities.tsv");
		
		spawning.write("model/output/spawning.tsv");
		biomass_spawning_unfished.write("model/output/biomass_spawning_unfished.tsv");
		
		movement.write("model/output/movement.tsv");

		numbers.write("model/output/numbers.tsv");

		selectivity_values.write("model/output/selectivity_values.tsv");
		selectivities.write("model/output/selectivities.tsv");
	}
};

}
