#pragma once

#include <array>

#include "spline.hpp"

#include "common.hpp"
#include "dimensions.hpp"
#include "distributions.hpp"

#define TRACKING 1

namespace IOSKJ {

/**
 * Model of the Indian Ocean skipjack tuna fishery. This class encapsulates both fish
 * population and fishing dynamics.
 * 
 * @author Nokome Bentley <nokome.bentley@trophia.com>
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
	 * This is recorded in step() so that the biomass_spawning_unfished
	 * can be set by init()
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
	 * Standard deviation of recruitment vaiation
	 */
	double recruits_sd;

	/**
	 * Flag to turn on/off recruitment variation
	 */
	bool recruits_variation_on;

	/**
	 * Flag to turn on/off recruitment relation
	 */
	bool recruits_relation_on;

	/**
	 * Deterministic recruitment at time t 
	 */
	double recruits_determ;

	/**
	 * Recruitment deviation (multiplier) at time t
	 */
	double recruits_deviation;

	/**
	 * Total number of recruits at time t
	 */
	double recruits;

	/**
	 * Recruitment by region
	 */
	Array<double,Region> recruits_regions;

	/**
	 * Initial sizes of recruits
	 */
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
	Array<double,Size> weights;
	
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
	Array<double,Size> maturities;

	/**
	 * @}
	 */
	
	/**
	 * @{
	 * @name Natural mortality
	 */
	
	/**
	 * Instantaneous rate of natural mortality
	 */
	double mortality;

	/**
	 * Quarterly rate of survival from natural mortality
	 */
	double natural_survival;

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
	 * @name Selectivity/exploitation
	 */
	
	typedef std::array<double,5> SelectivityPoints;
	Array<SelectivityPoints,Method> selectivity_points;

	Array<double,Method,Size> selectivities;

	/**
	 * A switch used to turn off/on exploitation dynamics
	 * (e.g turn off for virgin equilibrium)
	 */
	bool exploitation_on;

	/**
	 * Vulneralble biomass by region and method
	 */
	Array<double,Region,Method> biomass_vulnerable;

	/**
	 * Exploitation rate by region and size for current time step
	 */
	Array<double,Region,Size> exploitation;

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
				<<"period\t"
				<<"year\t"
				<<"quarter\t"
				<<"recruits_determ\t"
				<<"recruits_deviation\t"
				<<"recruits\t"
				<<"biomass_spawning_overall\t"
				<<"biomass_spawning_w\t"
				<<"biomass_spawning_m\t"
				<<"biomass_spawning_e"
				<<std::endl;
		}

		void track(void){
			if(track_on){
				track_file
					<<period<<"\t"
					<<year<<"\t"
					<<quarter<<"\t"
					<<recruits_determ<<"\t"
					<<recruits_deviation<<"\t"
					<<recruits<<"\t"
					<<biomass_spawning_overall(quarter)<<"\t"
					<<biomass_spawning(W)<<"\t"
					<<biomass_spawning(M)<<"\t"
					<<biomass_spawning(E)
					<<std::endl;
			}
		}

		void track_close(void){
			track_on = false;
			track_file.close();
		}

	#else

		void track_open(void){
		}

		void track(void){
		}

		void track_close(void){
		}

	#endif

	/**
	 * @}
	 */

	/**
	 * Perform any necessary steps at initial model execution 
	 */
	void startup(void){
	}

	//! @{
	//! @name Parameter setting methods

	/**
	 * Set default parameter values
	 *
	 * Mainly used in testing
	 */
	void defaults(void){
		recruits_unfished = 10e6;
		recruits_steepness = 0.9;
		recruits_sd = 0.6;

		recruits_regions = {0.5,0.3,0.2};

		recruits_sizes = 0.0;
		recruits_sizes[0] = 1;

		weight_a = 5.32e-6;
		weight_b = 3.35;

		maturity_inflection = 40;
		maturity_steepness = 5;

		mortality = 0.8;

		growth_rate = 0.3;
		growth_assymptote = 75;
		growth_sd = 1;
		growth_cv = 0.2;

		movement_pars(W,W) = 0.85;
		movement_pars(W,M) = 0.10;
		movement_pars(W,E) = 0.05;

		movement_pars(M,W) = 0.10;
		movement_pars(M,M) = 0.80;
		movement_pars(M,E) = 0.10;

		movement_pars(E,W) = 0.05;
		movement_pars(E,M) = 0.10;
		movement_pars(E,E) = 0.85;

		spawning = {0.8,0.5,0.8,0.5};

		selectivity_points(PS)[0] = 0.0;
		selectivity_points(PS)[1] = 0.1;
		selectivity_points(PS)[2] = 0.3;
		selectivity_points(PS)[3] = 0.5;
		selectivity_points(PS)[4] = 1.0;

		selectivity_points(PL)[0] = 0.0;
		selectivity_points(PL)[1] = 0.1;
		selectivity_points(PL)[2] = 0.3;
		selectivity_points(PL)[3] = 0.5;
		selectivity_points(PL)[4] = 1.0;

		selectivity_points(GN)[0] = 0.0;
		selectivity_points(GN)[1] = 0.1;
		selectivity_points(GN)[2] = 0.3;
		selectivity_points(GN)[3] = 0.5;
		selectivity_points(GN)[4] = 1.0;

		selectivity_points(LI)[0] = 0.0;
		selectivity_points(LI)[1] = 0.1;
		selectivity_points(LI)[2] = 0.3;
		selectivity_points(LI)[3] = 0.5;
		selectivity_points(LI)[4] = 1.0;

		selectivity_points(OT)[0] = 0.0;
		selectivity_points(OT)[1] = 0.1;
		selectivity_points(OT)[2] = 0.3;
		selectivity_points(OT)[3] = 0.5;
		selectivity_points(OT)[4] = 1.0;
	}

	/**
	 * Set recruitment distribution across regions to be uniform
	 */
	void recruits_uniform(void){
		recruits_regions = 1.0/recruits_regions.size();
	}
 
	/**
	 * Set movement parameters so that there is no movement.
	 *
	 * Diagonal elements set to 1. Mainly used for testing
	 */
	void movement_none(void){
		movement_pars = [](uint region_from, uint region_to){
			return double(region_from==region_to?1:0);
		};
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
	 * Initialise the fish population based on current parameter values
	 */
	void init(void){
		// Initialise arrays by size...
		for(auto size : sizes){

			lengths[size] = 2*size+1;

			weights[size] = weight_a * std::pow(lengths[size],weight_b);

			maturities[size] = 1/(1+std::pow(19,(maturity_inflection-lengths[size])/maturity_steepness));
		}

		// Initialise natural survival rate
		natural_survival = std::exp(-0.25*mortality);

		// Initialise growth size transition matrix
		for(auto size : sizes){
			growth_increments(size) = (growth_assymptote-lengths(size))*(1-std::exp(-0.25*growth_rate));
		}
		for(auto size_from : size_froms){
			double growth_increment = growth_increments(size_from);
			double mean = lengths(size_from) + growth_increment;
			double sd = std::pow(std::pow(growth_sd,2)+std::pow(growth_increment*growth_cv,2),0.5);
			for(auto size : sizes){
				growth(size_from,size) = normal_integral(2*size,2*(size+1),mean,sd);
			}
		}

		// Initialise movement matrix
		auto movement_sums = movement_pars(by(region_froms),sum());
		for(auto region_from : region_froms){
			for(auto region : regions){
				movement(region_from,region) = 
					movement_pars(region_from,region)/movement_sums(region_from);
			}
		}

		// Initialise selectivity
		for(auto method : methods){
			auto points = selectivity_points(method);
			Spline<double,double> selectivity_spline(
				{0,20,40,60,80},
				std::vector<double>(points.begin(),points.end())
			);

			for(auto size : sizes){
				selectivities(method,size) = selectivity_spline.interpolate(lengths(size));
			}
		}

		/**
		 * The fish population is initialised to an unfished state
		 * by iterating with virgin recruitment until it reaches equibrium
		 * defined by less than 0.01% change in total biomass.
		 */
		// Initialise the population to zero
		numbers = 0.0;
		// Turn off recruitment relationship, variation and exploitation
		recruits_relation_on = false;
		exploitation_on = false;
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

		write();
	}

	void step(void){

		// Calculate total biomass and spawning biomass by region
		for(auto region : regions){
			double biomass_ = 0;
			double biomass_spawning_ = 0;
			for(auto age : ages){
				for(auto size : sizes){
					double biomass = numbers(region,age,size) * weights(size);
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
			recruits_deviation = lognormal_rand(1,recruits_sd);
		} else {
			recruits_deviation = 1;
		}
		recruits = recruits_determ * recruits_deviation;

		// Ageing and recruitment
		for(auto region : regions){
			for(auto size : sizes){
				// Oldest age class accumulates 
				numbers(region,ages.size-1,size) += numbers(region,ages.size-2,size);

				// For most ages just "shuffle" along
				for(uint age=ages.size-2;age>0;age--){
					numbers(region,age,size) = numbers(region,age-1,size);
				}

				// Recruits are evenly distributed over regions and over sizes
				// according to `initials`
				numbers(region,0,size) = recruits * recruits_regions(region) * recruits_sizes(size);
			}
		}

		if(exploitation_on){
			// Exploitation rate
			for(auto region : regions){
				for(auto method : methods){
					double sum = 0;
					for(auto age : ages){
						for(auto size : sizes){
							sum += numbers(region,age,size) * weights(size) * selectivities(method,size);
						}
					}
					biomass_vulnerable(region,method) = sum;
				}
			}
			//exploitation_rate(region_method) = catches(region,method)/biomass_vulnerable(region,method);
		} else {

		}

		/*
		for(auto region : regions){
			for(auto size : sizes){
				double sum = 0;
				for(auto method : methods){
					for(auto age : ages){
						sum += 
					}
				}
				exploitation_survival(region,size) = 1-sum;
			}
		}
		*/
	
		// Mortality, growth and movement
		auto numbers_temp = numbers;
		for(auto region : regions){
			for(auto age : ages){
				for(auto size : sizes){
					double sum = 0;
					for(auto region_from : region_froms){
						for(auto size_from : size_froms){
							sum += numbers(region_from,age,size_from) * 
									growth(size_from,size) * 
									natural_survival/*(size_from)*/ * 
									//exploitation_survival(region_from,size_from) * 
									movement(region_from,region);
						}
					}
					numbers_temp(region,age,size) = sum;
				}
			}
		}
		numbers = numbers_temp;

		track();
	}

	void equilibrium(void){
		// Iterate until there is a very minor change in biomass

		// Turn off recruitment variation
		recruits_variation_on = false;

		uint steps = 0;
		const uint steps_max = 10000;
		Array<double,Region> biomass_prev = 1;
		while(steps<steps_max){
			for(quarter=0;quarter<4;quarter++) step();

			double diffs = 0;
			for(auto region : regions){
				diffs += fabs(biomass(region)-biomass_prev(region))/biomass_prev(region);
			}
			if(diffs<0.0001) break;
			biomass_prev = biomass;

			steps++;
		}
		// Throw an error if there was no convergence
		assert(steps<steps_max);

		// Turn on recruitment deviation again
		recruits_variation_on = true;
	}

	void simulate(uint begin,uint end){
		for(period=begin;period<end;period++){
			time(period);
			step();
		}
	}

	void write(void){
		numbers.write("output/numbers.txt");
		lengths.write("output/lengths.txt");
		weights.write("output/weights.txt");
		maturities.write("output/maturities.txt");
		spawning.write("output/spawning.txt");
		growth_increments.write("output/growth-increments.txt");
		growth.write("output/growth.txt");
		movement.write("output/movement.txt");
		selectivities.write("output/selectivities.txt");
	}

	void shutdown(void){
	}

};

}
