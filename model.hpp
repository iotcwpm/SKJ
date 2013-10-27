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

	double recruit_virgin;

	/**
	 * Unfished spawning biomass by quarter. It is necessary to have this by quarter
	 * because the proportion of mature fish that spawn varies by quarter.
	 */
	Array<double,Quarter> recruit_virgin_spawners;

	double recruit_steepness;
	double recruit_sd;
	bool recruit_variation_on;
	bool recruit_relation_on;
	Array<double,Region> recruit_regions;
	Array<double,Size> recruit_sizes;

	double recruits_determ;
	double recruits_deviation;
	double recruits;
	
	Array<double,Size> lengths;
	const double lengths_step = 2;

	double weight_a;
	double weight_b;
	Array<double,Size> weights;
	
	double maturity_inflection;
	double maturity_steepness;
	Array<double,Size> maturities;
	
	double mortality;
	double survival;

	double growth_rate;
	double growth_assymptote;
	double growth_sd;
	double growth_cv;
	Array<double,Size> growth_increments;
	Array<double,SizeFrom,Size> growth;

	Array<double,RegionFrom,Region> movement_pars;
	Array<double,RegionFrom,Region> movement;


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
	 *
	 * @see exploit
	 */
	Array<double,Region,Size> exploitation;

	/**
	 * The spawning fraction by quarter
	 */
	Array<double,Quarter> spawning;
	

	Array<double,Region> biomass;
	Array<double,Region> biomass_spawning;

	/**
	 * Spawning biomass in each of the most recent quarters
	 * This is recorded in step() so that the recruits_virgin_spawners
	 * can be set by init()
	 */
	Array<double,Quarter> biomass_spawning_overall;

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
		recruit_virgin = 10e6;
		recruit_steepness = 0.9;
		recruit_sd = 0.6;

		recruit_regions = {0.5,0.3,0.2};

		recruit_sizes = 0.0;
		recruit_sizes[0] = 1;

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
	void recruit_uniform(void){
		recruit_regions = 1.0/recruit_regions.size();
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
		survival = std::exp(-0.25*mortality);

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
		recruit_relation_on = false;
		exploitation_on = false;
		// Go to equilibrium
		equilibrium();
		// Turn on recruitment relationship etc again
		recruit_relation_on = true;
		exploitation_on = true;

		/**
		 * Once the population has converged to unfished equilibrium, the virgin
		 * spawning biomass in each quarter can be set.
		 */
		for(auto quarter : quarters) recruit_virgin_spawners(quarter) = biomass_spawning_overall(quarter);

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
		if(recruit_relation_on){
			// Stock-recruitment realtion is active so calculate recruits based on 
			// the spawning biomass in the previous time step
			//! @todo check this equation
			recruits_determ = 4 * recruit_steepness * recruit_virgin * biomass_spawning_overall(quarter) / (
				(5*recruit_steepness-1)*biomass_spawning_overall(quarter) + 
				recruit_virgin_spawners(quarter)*(1-recruit_steepness)
			);
		} else {
			// Stock-recruitment relation is not active so recruitment is just r0.
			recruits_determ = recruit_virgin;
		}
		if(recruit_variation_on){
			recruits_deviation = lognormal_rand(1,recruit_sd);
		} else {
			recruits_deviation = 1;
		}
		recruits = recruits_determ * recruits_deviation;

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
		}

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
				numbers(region,0,size) = recruits * recruit_regions(region) * recruit_sizes(size);
			}
		}

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
									survival * 
									//(1-(exploitation_on?exploitation(region_from,size_from):0)) * 
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
		recruit_variation_on = false;

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

		std::cout<<steps<<"\n";

		// Turn on recruitment deviation again
		recruit_variation_on = true;
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
