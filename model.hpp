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
	 * Fish numbers by region and age
	 */
	Array<double,Region,Age> numbers;

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
	 * @}
	 */

	/**
	 * @{
	 * @name Growth
	 */
	
	/**
	 * Parameters of the two-stanza vonBertallanffy
	 */
	double growth_rate_1;
	double growth_rate_2;
	double growth_assymptote;
	double growth_stanza_inflection;
	double growth_stanza_steepness;
	double growth_age_0;
	double growth_cv_0;
	double growth_cv_old;

	/**
	 * Length associated with each size
	 */
	Array<double,Size> length_size;

	/**
	 * Length distribution for each age group
	 */
	Array<Normal,Age> length_age;

	/**
	 * Proportion of fish of each age in each size bin
	 */
	Array<double,Age,Size> age_size;

	/**
	 * @}
	 */
	
	/**
	 * @{
	 * @name Weight
	 */

	/**
	 * Weight at length power funciton
	 */
	double weight_length_a;
	double weight_length_b;

	/**
	 * Weight at size
	 */
	Array<double,Size> weight_size;
	Array<double,Age> weight_age;

	/**
	 * @}
	 */
	
	/**
	 * @{
	 * @name Maturity
	 */
	
	/**
	 * Maturity at length logistic function
	 */
	double maturity_length_inflection;
	double maturity_length_steepness;

	/**
	 * Maturity at size and age
	 */
	Array<double,Size> maturity_size;
	Array<double,Age> maturity_age;

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
	 * Age (in quarters) with maximum rate of natural mortality (exponential relationship
	 * can give very high natural mortality at small ages).
	 */
	int mortality_cap_age = 2; // i.e. 2+ quarters 

	/**
	 * Instantaneous rate of natural mortality for size s
	 */
	Array<double,Age> mortality;

	/**
	 * Quarterly rate of survival from natural mortality for size s
	 */
	Array<double,Age> survival;

	/**
	 * @}
	 */


	/**
	 * @{
	 * @name Movement
	 */
	
	/**
	 * Movement maximum proportion moving from one region to another
	 */
	Array<double,RegionFrom,Region> movement_region;
	
	/**
	 * Movement proportion at size logistic function
	 */
	double movement_length_inflection;
	double movement_length_steepness;
	Array<double,Size> movement_size;
	Array<double,Age> movement_age;

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
	Array<double,Method,Size> selectivity_size;
	Array<double,Method,Age> selectivity_age;

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
	 * Escapement (i.e. survival form exploitation)
	 */
	Array<double,Region,Age> escapement;

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
	 * All movement_regions elements set to 1/(number of regions). All sizes move.
	 * Mainly used for testing
	 */
	void movement_uniform(void){
		movement_region = 1.0/regions.size();
		movement_length_inflection = 0;
		movement_length_steepness = 100;
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
		exploitation_rate_specified(SW,PS) = value;
		exploitation_rate_specified(NW,PS) = value;
		exploitation_rate_specified(MA,PL) = value;
		exploitation_rate_specified(EA,GN) = value;
	}

	/**
	 * Get overall exploitation rate.
	 */
	double exploitation_rate_get(void) const {
		double survival = escapement(geomean);
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
		// Initialise length distributions by age
		double growth_cv_slope = (growth_cv_old - growth_cv_0)/ages.size();
		for(auto age : ages){
			// Convert age from quarters (middle of quarter) to years
			double a = double(age.index()+0.5)/4.0;
			// Mean length at age
			double part1 = (1+std::exp(-growth_stanza_steepness*(a-growth_age_0-growth_stanza_inflection)))/
						   (1+std::exp(growth_stanza_inflection*growth_stanza_steepness));
			double part2 = std::pow(part1,-(growth_rate_2-growth_rate_1)/growth_stanza_steepness);
			double mean = growth_assymptote * (1-std::exp(-growth_rate_2*(a-growth_age_0))*part2);
			// Standard deviation of length at age
			double cv = growth_cv_0 + growth_cv_slope*a;
			double sd = mean * cv;
			Normal dist(mean,sd);
			length_age(age) = dist;
			// Calculate proportions in each size bin
			double sum = 0;
			for(auto size : sizes){
				double lower = 2*size.index();
				double upper = lower+2;
				double prop = dist.cdf(upper)-dist.cdf(lower);
				age_size(age,size) = prop;
				sum += prop;
			}
			// Normalise to ensure rows sum to 1
			for(auto size : sizes) age_size(age,size) /= sum;
		}

		// Initialise arrays that are dimensioned by size	
		for(auto size : sizes){
			double length = 2*size.index()+1;
			length_size(size) = length;
			weight_size(size) = weight_length_a*std::pow(length,weight_length_b);
			maturity_size(size) = 1.0/(1.0+std::pow(19,(maturity_length_inflection-length)/maturity_length_steepness));
			movement_size(size) = 1.0/(1.0+std::pow(19,(movement_length_inflection-length)/movement_length_steepness));
		}

		// Initialise selectivity
		for(auto method : methods){
			// Iterpolate selectivity-at-size using piecewise spline
			// Ensure that selectivity is between 0 and 1 since spline
			// can produce values outside this range even if knots are not
			double max = 0;
			for(auto size : sizes){
				double length = length_size(size);
				double selectivity = 0;
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
				selectivity_size(method,size) = selectivity;
			}
			for(auto size : sizes) selectivity_size(method,size) /= max;
		}
		
		// Initialise arrays that are dimensioned by age but dependent
		// up arrays dimensioned by size
		weight_age = 0;
		maturity_age = 0;
		selectivity_age = 0;
		for(auto age : ages){
			for(auto size : sizes){
				weight_age(age) += weight_size(size) * age_size(age,size);
				maturity_age(age) += maturity_size(size) * age_size(age,size);
				movement_age(age) += movement_size(size) * age_size(age,size);
				for(auto method : methods) selectivity_age(method,age) += selectivity_size(method,size) * age_size(age,size);
			}
		}

		// Normalise movement and selectivities to maximum
		double movement_max = -1;
		Array<double,Method> selectivity_max = -1;
		for(auto age : ages){
			if(movement_age(age)>movement_max) movement_max = movement_age(age);
			for(auto method : methods){
				if(selectivity_age(method,age)>selectivity_max(method)) selectivity_max(method) = selectivity_age(method,age);
			}
		}
		for(auto age : ages){
			movement_age(age) /= movement_max;
			for(auto method : methods) selectivity_age(method,age) /= selectivity_max(method);
		}

		// Set up mortality by age schedule
		for(auto age : ages){
			double weight = (age<mortality_cap_age)?weight_age(mortality_cap_age):weight_age(age);
			mortality(age) = mortality_base * std::pow(weight,mortality_exponent);
			survival(age) = std::exp(-0.25*mortality(age));
		}

		// Initialise regional movement matrix
		for(auto region_from : region_froms){
			// Check that the off diagonal elements sum to between 0 and 1
			double off_diagonals = 0;
			for(auto region : regions){
				if(region_from.index()!=region.index()) off_diagonals += movement_region(region_from,region);
			}
			// If they don't then normalise them
			if(off_diagonals<0){
				throw std::runtime_error("Woahhh! There is a negative regional movement parameter");
			}
			else if(off_diagonals>1){
				for(auto region : regions){
					if(region_from.index()!=region.index()) movement_region(region_from,region) = movement_region(region_from,region)/off_diagonals;
				}
				off_diagonals = 1;
			}
			// Ensure diagonals are complements
			movement_region(region_from,Level<Region>(region_from)) = 1 - off_diagonals;
		}

		// Normalise the recruits_region grid so that it sums to one
		recruits_regions /= sum(recruits_regions);

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
				double biomass = numbers(region,age) * weight_age(age)/1000;
				biomass_ += biomass;
				double spawners = biomass * maturity_age(age);
				biomass_spawners_ += spawners;
				biomass_spawning_ += spawners * spawning(quarter);
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
			// Oldest age class accumulates 
			numbers(region,ages.size()-1) += numbers(region,ages.size()-2);

			// For most ages just "shuffle" along
			for(uint age=ages.size()-2;age>0;age--){
				numbers(region,age) = numbers(region,age-1);
			}

			// Recruits are evenly distributed over regions and over sizes
			// according to `initials`
			numbers(region,0) = recruits * recruits_regions(region);
		}

		// Natural mortality
		for(auto region : regions){
			for(auto age : ages){
				numbers(region,age) *= survival(age);
			}
		}

		// Movement
		for(auto region_from : region_froms){
			for(auto region_to : regions){
				for(auto age : ages){
					double movers = movement_region(region_from,region_to) * movement_age(age);
					numbers(Level<Region>(region_from),age) -= movers;
					numbers(region_to,age) += movers;
				}
			}
		}

		// Fishing mortality
		if(exploit!=exploit_none){
			// For each region and method
			for(auto region : regions){
				for(auto method : methods){
					// Calculate vulnerable biomass
					double biomass_vuln = 0;
					for(auto age : ages){
						biomass_vuln += numbers(region,age) * 
						                weight_age(age)/1000 * 
							            selectivity_age(method,age);
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
			// Pre-calculate the escapement for each region and ages
			for(auto region : regions){
				for(auto age : ages){
					double proportion_taken = 0;
					for(auto method : methods){
						proportion_taken += exploitation_rate(region,method) * selectivity_age(method,age);
					}
					escapement(region,age) = (proportion_taken>1)?0:(1-proportion_taken);
				}
			}
		} else {
			escapement = 1;
		}
		// Apply escapement
		for(auto region : regions){
			for(auto age : ages){
				numbers(region,age) *= escapement(region,age);
			}
		}
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
				std::cout<<steps<<"\t"<<biomass(SW)<<"\t"<<biomass(NW)<<"\t"<<biomass(MA)<<"\t"<<biomass(EA)<<"\t"<<diffs<<std::endl;
			#endif

			// Throw an error if undefined biomass
			if(not std::isfinite(biomass(SW)+biomass(NW)+biomass(MA)+biomass(EA))){
				write();
				throw std::runtime_error("Biomass is not finite. Check inputs. Model has been written to `model/output`");
			}

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
		Frame curve({
			"exprate","f","yield","status","vuln",
			"catch_sw_ps","catch_nw_ps","catch_ma_pl","catch_ea_gn",
			"vuln_sw_ps","vuln_nw_ps","vuln_ma_pl","vuln_ea_gn"
		});
		for(double exprate=0;exprate<1;exprate+=step){
			#if DEBUG
				std::cout<<"************yield_curve "<<exprate<<"**************\n";
			#endif
			exploitation_rate_set(std::max(exprate,1e-6));
			equilibrium();
			curve.append({
				exprate,fishing_mortality_get(),catches_taken(sum),biomass_status(0),biomass_vulnerable(sum),
				catches_taken(SW,PS),catches_taken(NW,PS),catches_taken(MA,PL),catches_taken(EA,GN),
				biomass_vulnerable(SW,PS),biomass_vulnerable(NW,PS),biomass_vulnerable(MA,PL),biomass_vulnerable(EA,GN)
			});
		}
		return curve;
	}

	/**
	 * Generate a yield per recruit curve
	 *
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
				double n = numbers(region,age);
				number += n;
				// TODO
				//length += length_age(size) * n;
				//weight += weights_age(size) * n;
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
		numbers.write("model/output/numbers.tsv");
		spawning.write("model/output/spawning.tsv");
		biomass_spawning_unfished.write("model/output/biomass_spawning_unfished.tsv");
		
		recruits_regions.write("model/output/recruits_regions.tsv");
		
		length_size.write("model/output/length_size.tsv");
		length_age.write("model/output/length_age.tsv",{"mean","sd"},[](std::ostream& stream,const Normal& dist){
			stream<<dist.mean<<"\t"<<dist.sd;
		});
		age_size.write("model/output/age_size.tsv");

		weight_size.write("model/output/weight_size.tsv");
		weight_age.write("model/output/weight_age.tsv");

		maturity_size.write("model/output/maturity_size.tsv");
		maturity_age.write("model/output/maturity_age.tsv");

		mortality.write("model/output/mortality.tsv");
		survival.write("model/output/survival.tsv");
		
		movement_region.write("model/output/movement_region.tsv");
		movement_size.write("model/output/movement_size.tsv");
		movement_age.write("model/output/movement_age.tsv");

		selectivity_size.write("model/output/selectivity_size.tsv");
		selectivity_age.write("model/output/selectivity_age.tsv");

		catchability.write("model/output/catchability.tsv");
	}
};

}
