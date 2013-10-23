#pragma once

#include "common.hpp"
#include "dimensions.hpp"
#include "distributions.hpp"

namespace IOSKJ {

class Fish {
public:

	Array<double,Region,Age,Size> numbers;

	double recruit_virgin;
	double recruit_steepness;
	double recruit_sd;
	bool recruit_variation;
	bool recruit_relation;
	Array<double,Region> recruit_regions;
	Array<double,Size> recruit_sizes;
	
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

	/**
	 * Exploitation rate by region and size for current time step
	 *
	 * @see exploit
	 */
	Array<double,Region,Size> exploitation;

	Array<double,Quarter> spawning;
	double spawners;
	double spawners_virgin;

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

		recruit_sizes = 0;
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

		movement_pars(W,M) = 0.10;
		movement_pars(W,E) = 0.05;
		movement_pars(M,W) = 0.10;
		movement_pars(M,E) = 0.10;
		movement_pars(E,M) = 0.10;
		movement_pars(E,W) = 0.05;


		spawning = {0.8,0.5,0.8,0.5};
	}

	/**
	 * Sample parameter values from prior 
	 * probabiity distributions
	 */
	void sample(void){
	}

	/**
	 * Initialise the fish population with 
	 * based on current parameter values
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
		auto movement_sums = movement(by(region_froms),sum());
		for(auto region_from : region_froms){
			for(auto region : regions){
				movement(region_from,region) = movement(region_from,region)/movement_sums(region_from);
			}
		}

		/**
		 * The fish population is initialised to an unfished state
		 * by iterating with virgin recruitment until it reaches equibrium
		 * defined by less than 0.01% change in total biomass
		 */
		// Initialise the population to zero
		numbers = 0;
		// Turn off recruitment relationship
		recruit_relation = false;
		// Go to equilibrium
		equilibrium();
		// Turn on recruitment relationship again
		recruit_relation = true;

		write();
	}

	void step(void){

		// Recruits
		double recruits;
		if(recruit_relation){
			// Stock-recruitment realtion is active so calculate recruits based on 
			// the spawning biomass in the previous time step
			//! @todo check this equation
			recruits = 4 * recruit_steepness * recruit_virgin * spawners / (
				(5*recruit_steepness-1)*spawners + spawners_virgin*(1-recruit_steepness)
			);
		}
		else {
			// Stock-recruitment realtion is not active so recruitment is just r0.
			recruits = recruit_virgin;
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
									//(1-exploitation(region_from,size_from)) * 
									movement(region_from,region);
						}
					}
					numbers_temp(region,age,size) = sum;
				}
			}
		}
		numbers = numbers_temp;

		// Spawners
		spawners = 0;
		for(auto region : regions){
			for(auto age : ages){
				for(auto size : sizes){
					spawners = numbers(region,age,size) * maturities(size) * weights(size) * spawning(quarter);
				}
			}
		}
	}

	double biomass(void) const {
		double sum = 0;
		for(auto region : regions){
			for(auto age : ages){
				for(auto size : sizes){
					sum += numbers(region,age,size) * weights(size);
				}
			}			
		}
		return sum;
	}

	double equilibrium(void){
		// Iterate until there is a very minor change in biomass

		// Turn off recruitment variation
		recruit_variation = false;

		unsigned int steps = 0;
		double biomass_prev = 1;
		while(steps<10000){
			for(quarter=0;quarter<4;quarter++) step();

			double biomass_curr = biomass();
			#if DEBUG
				std::cout<<steps<<"\t"<<biomass_curr<<"\t"<<biomass_prev<<std::endl;
			#endif
			if(fabs(biomass_curr-biomass_prev)/biomass_prev<0.0001) break;
			biomass_prev = biomass_curr;
			steps++;
		}

		// Turn on recruitment deviation again
		recruit_variation = true;

		return biomass_prev;
	}

	void write(void){
		numbers.write("output/fish-numbers.txt");
		lengths.write("output/fish-lengths.txt");
		weights.write("output/fish-weights.txt");
		maturities.write("output/fish-maturities.txt");
		spawning.write("output/fish-spawning.txt");
		growth_increments.write("output/fish-growth-increments.txt");
		growth.write("output/fish-growth.txt");
		movement.write("output/fish-movement.txt");
	}

};

}
