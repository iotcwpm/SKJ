#pragma once

#include "dimensions.hpp"
#include "distributions.hpp"
using namespace Utilities::Distributions;

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
	 * Total biomass of spawners
	 */
	Array<double,Region> biomass_spawners;
	
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
	 * Unfished equlibrium spawners (biomass)
	 *
	 * This differs from `biomass_spawning_unfished` in that it is not affected by the proportion
	 * spawning in a season
	 */
	double biomass_spawners_unfished;

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
	 * 
	 */
	enum {
		// For determining pristine conditions
		exploit_none = 0,
		// For determining MSY related reference points
		// and F based MPs
		exploit_rate = 1,
		// For conditioning with historical catches and
		// TAC based MPs
		exploit_catch = 2,
		// For TAE based MPs
		exploit_effort = 3,
	} exploit;

	/**
	 * Vulnerable biomass by region and method
	 */
	Array<double,Region,Method> biomass_vulnerable;

	/**
	 * CPUE. Simply `biomass_vulnerable` scaled to
	 * its geometric mean for the period 1985-2013
	 */
	Array<double,Region,Method> cpue;
	Array<GeometricMean,Region,Method> cpue_base;

	/**
	 * Catches by region and method
	 */
	Array<double,Region,Method> catches;

	/**
	 * Effort by region and method
	 *
	 * Currently these are nominal units relative
	 * to the period 2004-2013
	 */
	Array<double,Region,Method> effort;

	/**
	 * Estimated catchability by region and method
	 */
	Array<double,Region,Method> catchability;
	Array<GeometricMean,Region,Method> catchability_estim;


	/**
	 * The exploitation rate specified, for example, when calculating MSY/Bmsy
	 */
	Array<double,Region,Method> exploitation_rate_specified;

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
	 * MSY related variables
	 */
	double msy;
	double e_msy;
	double f_msy;
	double biomass_spawners_msy;
	int msy_trials;

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
	double biomass_status(uint time) const{
		return biomass_spawners(sum)/biomass_spawners_unfished;
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
	 * Set overall exploitation rate. Used in testing and in 
	 * equilibrium exploitation e.g. MSY/BMSY calculations
	 *
	 * Currently assumes an equal explotation rate is applied to the
	 * three main methods in each region (W/PS,M/PL,E/GN).
	 * An alternative would be to assume equal exploitation rates
	 * in each region and use current partial exploitation rates by method
	 * within each region.
	 */
	void exploitation_rate_set(double value){
		exploit = exploit_rate;
		// Set exploitation rate to be zero for most areas
		// but to `value` for the three main methods in each
		// region.
		exploitation_rate_specified = 0;
		exploitation_rate_specified(W,PS) = value;
		exploitation_rate_specified(M,PL) = value;
		exploitation_rate_specified(E,GN) = value;
	}

	/**
	 * Get overall exploitation rate.
	 */
	double exploitation_rate_get(void) const {
		double survival = exploitation_survival(geomean);
		return 1 - survival;
	}

	/**
	 * Set overall instantaneous rate of fishing mortality (F).
	 * Like `exploitation_rate_set` but uses F instead of exp. rate.
	 */
	void fishing_mortality_set(double value){
		exploitation_rate_set(1-std::exp(-value));
	}

	/**
	 * Get overall instantaneous rate of fishing mortality (F).
	 * Like `exploitation_rate_get` but gives F instead of exp. rate.
	 */
	double fishing_mortality_get(void) const {
		return -std::log(1-exploitation_rate_get());
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
			weights(size) = weight_length_a*std::pow(length,weight_length_b);
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

		// Go to pristine
		pristine_go();

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

		// Calculate total biomass, spawners biomass and spawning biomass by region
		for(auto region : regions){
			double biomass_ = 0;
			double biomass_spawners_ = 0;
			double biomass_spawning_ = 0;
			for(auto age : ages){
				for(auto size : sizes){
					double biomass = numbers(region,age,size) * weights(size)/1000;
					biomass_ += biomass;
					double spawners = biomass * maturities(size);
					biomass_spawners_ += spawners;
					biomass_spawning_ += spawners * spawning(quarter);
				}
			}
			biomass(region) = biomass_;
			biomass_spawners(region) = biomass_spawners_;
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

		if(exploit!=exploit_none){
			// For each region and method
			for(auto region : regions){
				for(auto method : methods){
					// Calculate vulnerable biomass
					double biomass_vuln = 0;
					for(auto age : ages){
						for(auto size : sizes){
							biomass_vuln += numbers(region,age,size) * 
							                weights(size)/1000 * 
								            selectivities(method,size);
						}
					}
					biomass_vulnerable(region,method) = biomass_vuln;

					// Update CPUE
					if(quarter==0){
						if(year==1985) cpue_base(region,method).reset();
						if(year>=1985 and year<=2013){
							cpue_base(region,method).append(biomass_vuln);
						} else {
							cpue(region,method) = biomass_vuln/cpue_base(region,method);
						}
					}

					// Determine exploitation rate
					double er = 0;
					if(exploit==exploit_catch){
						// Calculate exploitation rate from catches and biomass_vulnerable
						double c = catches(region,method);
						if(c>0){
							if(biomass_vuln>0){
								er = c/biomass_vuln;
								if(er>1) er = 1;
							} else er = 1;
						} else er = 0;
						// Update estimate of catchability
						double e = effort(region,method);
						if(e>0){
							double q = er/e;
							if(year==2004) catchability_estim(region,method).reset();
							if(q>0 and year>=2004 and year<=2013) catchability_estim(region,method).append(q);
							if(year==2013){
								catchability(region,method) = catchability_estim(region,method);
								// Where no catches for a region method make catchability 0
								if(not std::isfinite(catchability(region,method))){
									catchability(region,method) = 0;
								}
							}
						}
					}
					else if(exploit==exploit_effort){
						// Calculate exploitation rate from number of
						// effort units
						er = catchability(region,method) * effort(region,method);
					} else {
						// Use the exploitation rate specified
						er = exploitation_rate_specified(region,method);
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
					exploitation_survival(region,size) = (proportion_taken>1)?0:(1-proportion_taken);
				}
			}
		} else {
			exploitation_survival = 1;
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
										exploitation_survival(rf,sf) * 
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
		// Seed the population with a small population in each partition
		numbers = 1;	
		// Iterate until there is a very minor change in biomass
		uint steps = 0;
		const uint steps_max = 1000;
		Array<double,Region> biomass_prev = 1;
		while(steps<steps_max){
			// It is necessary to update the model for each quarter so that quarterly differences
			// in dynamics (e.g. spawning proportion) are incorporated
			for(uint quarter=0;quarter<4;quarter++) update(quarter);

			// Break if biomass has gone to very low levels (as happens when this method
			// is called with high exploitation rates from yield_curve) since the proportional
			// diffs minimise to a low level
			if(biomass(sum)<0.01) break;

			double diffs = 0;
			for(auto region : regions){
				diffs += fabs(biomass(region)-biomass_prev(region))/biomass_prev(region);
			}
			diffs /= regions.size();
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

	void pristine_go(void){
		// Calculate unfished state
		// Turn off recruitment relationship and exploitation
		recruits_relation_on = false;
		exploit = exploit_none;
		// Set unfished recruitment to an arbitrarily high number so it can be calculated in terms of biomass_spawners_unfished
		recruits_unfished = 1e10;
		// Go to equilibrium
		equilibrium();
		// Scale up unfished recruitment and biomass_spawning_unfished (by quarter) to match biomass_spawners_unfished
		double scalar = biomass_spawners_unfished/biomass_spawners(sum);
		recruits_unfished *= scalar;
		numbers *= scalar;
		for(auto quarter : quarters) biomass_spawning_unfished(quarter) = biomass_spawning_overall(quarter)*scalar;

		// Calculate the 
		// Turn on recruitment relationship etc again
		recruits_relation_on = true;
		exploit = exploit_catch;
	}

	/**
	 * Generate a yield curve
	 */
	Frame yield_curve(double step = 0.05){
		Frame curve({"exprate","f","yield","depletion"});
		curve.append({0,0,0,1});
		for(double exprate=step;exprate<1;exprate+=step){
			#if DEBUG
				std::cout<<"************yield_curve "<<exprate<<"**************\n";
			#endif
			exploitation_rate_set(exprate);
			equilibrium();
			double f = fishing_mortality_get();
			double yield = catches_taken(sum);
			double depletion = biomass_spawning_overall(sum)/biomass_spawning_unfished(sum);
			curve.append({exprate,f,yield,depletion});
		}
		return curve;
	}

	/**
	 * Generate a yield per recruit curve
	 *
	 * Since the model uses a size based transition matrix for growth
	 * take the population to equilibrium and then calculate numbers, 
	 * weight etc in each age class. 
	 * This is really a biomass-per-recruit curve.
	 */
	Frame yield_per_recruit(void){
		pristine_go();
		Frame ypr({"age","number","length","weight"});
		for(auto age : ages){
			double number = 0;
			double length = 0;
			double weight = 0;
			for(auto region : regions){
				for(auto size : sizes){
					double n = numbers(region,age,size);
					number += n;
					length += lengths(size) * n;
					weight += weights(size) * n;
				}
			}
			length /= number;
			weight /= number;
			ypr.append({double(age.index()),number,length,weight});
		}
		return ypr;
	}

	/**
	 * Take this model to a equilibirum state associated with MSY 
	 */
	void msy_go(void){
		int count = 0;
		auto result = boost::math::tools::brent_find_minima([&](double exprate){
			count++;
			exploitation_rate_set(exprate);
			equilibrium();
			return -catches_taken(sum);
		},0.01,0.99,8);
		e_msy = result.first;
		f_msy = -std::log(1-e_msy);
		msy = -result.second;
		msy_trials = count;
		// Go to equilibrium with maximum so that Bmsy can be determined
		exploitation_rate_set(e_msy);
		equilibrium();
		biomass_spawners_msy = biomass_spawners(sum);
	}

	/**
	 * Calculate MSY related reference points
	 */
	void msy_find(void){
		// Create a copy of this model and take it
		// to MSY
		Model calc = *this;
		calc.msy_go();
		// Copy over values
		e_msy = calc.e_msy;
		f_msy = calc.f_msy;
		msy = calc.msy;
		msy_trials = calc.msy_trials;
		biomass_spawners_msy = calc.biomass_spawners_msy;
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

		catchability.write("model/output/catchability.tsv");
	}
};

}
