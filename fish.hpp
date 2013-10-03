#pragma once

#include "common.hpp"
#include "dimensions.hpp"

namespace IOSKJ {

class Fish {
public:

	Array<double,Region,Age,Size> numbers;
	
	double r0;
	double recruit_sd;
	bool recruit_var;
	bool recruit_rel;

	Array<double,Size> initials;
	
	Array<double,Size> lengths;
	const double lengths_step = 2;

	double weight_a;
	double weight_b;
	Array<double,Size> weights;
	
	double maturity_50;
	double maturity_95;
	Array<double,Size> maturities;
	
	Array<double,Region,Age> mortalities;

	Array<double,Region,Age,RegionTo> movement;

	/**
	 * Set default parameter values
	 *
	 * Mainly used in testing
	 */
	void defaults(void){
		r0 = 10e6;

		initials = 0;
		initials[0] = 1;

		weight_a = 0.001;
		weight_b = 3;

		maturity_50 = 10;
		maturity_95 = 5;

		mortalities = 0.8;

		movement = 0.5;
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

		for(auto size : sizes){
			lengths[size] = size*2.0+0.5;

			weights[size] = weight_a * std::pow(lengths[size],weight_b);

			/**
			 * Maturity is a logistic function of length
			 */
			maturities[size] = maturity_50 * lengths[size] - maturity_95;//@todo
		}

		/**
		 * The fish population is initialised to an unfished state
		 * by iterating with virgin recruitment until it reaches equibrium
		 * defined by less than 0.01% change in total biomass
		 */
		// Initialise the population to zero
		numbers = 0;
		// Turn off recruitment relationship
		recruit_rel = false;
		// Go to equilibrium
		equilibrium();
		// Turn on recruitment relationship again
		recruit_rel = true;
	}

	void step(void){

		// Recruitment and aging
		if(quarter==1){
			//Calculate number of recruits
			double recruits;
			if(recruit_rel){
				//@todo
			}
			else {
				recruits = r0;
			}

			for(auto region : regions){
				for(auto size : sizes){
					numbers(region,ages.size-1,size) += numbers(region,ages.size-2,size);

					for(uint age=ages.size-2;age>0;age--){
						numbers(region,age,size) = numbers(region,age-1,size);
					}

					/**
					 * Recruits are evenly distributed over regions
					 */
					numbers(region,0,size) = recruits/regions.size * initials(size);
				}
			}

		}

		for(auto region : regions){
			for(auto age : ages){
				//double survival = survivals(region,age);
				for(auto size : sizes){
					for(auto region_to : region_tos){

					}
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
		recruit_var = false;

		unsigned int steps = 0;
		double biomass_prev = 1;
		while(steps<10000){
			for(quarter=1;quarter<=4;quarter++) step();

			double biomass_curr = biomass();
			#if DEBUG
				std::cout<<steps<<"\t"<<biomass_curr<<"\t"<<biomass_prev<<std::endl;
			#endif
			if(fabs(biomass_curr-biomass_prev)/biomass_prev<0.0001) break;
			biomass_prev = biomass_curr;
			steps++;
		}

		// Turn on recruitment deviation again
		recruit_rel = true;

		return biomass_prev;
	}

	void write(void){
		std::ofstream file("output/fish.txt");
		file<<"#numbers\n"<<numbers<<std::endl;
		file<<"#lengths\n"<<lengths<<std::endl;
		file<<"#weights\n"<<weights<<std::endl;
		file<<"#maturities\n"<<maturities<<std::endl;
	}

};

}
