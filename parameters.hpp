#pragma once

#include "model.hpp"

namespace IOSKJ {

/**
 * Class for defining model parameters, their priors, transformations etc.
 * See the `set()` method which "binds" parameters defined here to model variables.
 */
class Params : public ParameterSet<Params> {
public:

	typedef ParameterSet<Params> Base;
	
	/**
	 * Parameters of the stock-recruitment relationship
	 */
	Parameter<Uniform> recruits_unfished = {21,15,25};
	Parameter<Uniform> recruits_steepness = {0.9,0.7,1.0};
	Parameter<Uniform> recruits_sd = {0.6,0.4,0.8};

	/**
	 * Recruitment deviations
	 */
	Parameters<Truncated<Normal>,RecdevYear> recruits_deviations;

	/**
	 * Proportion of mature fish spawning by quarter
	 */
	Parameter<Uniform> spawning_0 = {0.8,0.7,1};
	Parameter<Uniform> spawning_1 = {0.5,0.3,0.7};
	Parameter<Uniform> spawning_2 = {0.8,0.7,1};
	Parameter<Uniform> spawning_3 = {0.5,0.3,0.7};

	/**
	 * Proportion of recruits by region
	 * Prior same for all regions. These are normalised in the model initialisation
	 * so that they sum to one.
	 */
	Parameter<Uniform> recruits_regions = {0.5,0.1,0.9};

	/**
	 * Parameters of the distribution of the lengths of recruits
	 */
	Parameter<Uniform> recruits_lengths_mean = {5,1,10};
	Parameter<Uniform> recruits_lengths_cv = {0.15,0.1,0.2};

	/**
	 * Length-weight parameters
	 */
    Parameter<Uniform> weight_a = {5.32e-6,5.32e-6,5.32001e-6};
    Parameter<Uniform> weight_b = {3.35,3.35,3.35001};

    /**
     * Maturity parameters
     */
    Parameter<Uniform> maturity_inflection = {42,40,45};
    Parameter<Uniform> maturity_steepness = {5,4,6};

    /**
     * Mortality parameters
     */
   	Parameter<Uniform> mortality_base = {0.7,0.5,0.9};
	Parameter<Fixed> mortality_exponent = -0.29;

    /**
     * Growth rate parameters
     */
	Parameter<Fixed> growth_rate = 0.3;
	Parameter<Fixed> growth_assymptote = 75;
	Parameter<Fixed> growth_sd = 1;
	Parameter<Fixed> growth_cv = 0.2;

    /**
     * Movements parameters
     *
     * There are 9 movement parameters (3 x 3 regions) but they are parameterised
     * into 4 priors. `movement_stay` defines a prior on the proportion of fish
     * that remain in an area.
     */
	Parameter<Uniform> movement_stay = {0.8,0.6,1.0};
	Parameter<Uniform> movement_w_m = {0.2,0.0,0.4};
	Parameter<Uniform> movement_m_e = {0.2,0.0,0.4};
	Parameter<Uniform> movement_w_e = {0.1,0.0,0.2};

	/**
	 * Selectivity parameters
	 */
	Parameters<Uniform,SelectivityKnot> selectivities;

	/**
	 * Catches by year, quarter, region and method
	 */
	Parameters<Fixed,Year,Quarter,Region,Method> catches;

    /**
     * Reflection
     */
    template<class Mirror>
    void reflect(Mirror& mirror){
        mirror
            .data(recruits_unfished,"recruits_unfished")
            .data(recruits_steepness,"recruits_steepness")
            .data(recruits_sd,"recruits_sd")
            .data(recruits_deviations,"recruits_deviations")

            .data(spawning_0,"spawning_0")
            .data(spawning_1,"spawning_1")
            .data(spawning_2,"spawning_2")
            .data(spawning_3,"spawning_3")

            .data(recruits_lengths_mean,"recruits_lengths_mean")
            .data(recruits_lengths_cv,"recruits_lengths_cv")

            .data(weight_a,"weight_a")
            .data(weight_b,"weight_b")

            .data(maturity_inflection,"maturity_inflection")
            .data(maturity_steepness,"maturity_steepness")

            .data(mortality_base,"mortality_base")
            .data(mortality_exponent,"mortality_exponent")

            .data(growth_rate,"growth_rate")
            .data(growth_assymptote,"growth_assymptote")
            .data(growth_sd,"growth_sd")
            .data(growth_cv,"growth_cv")

            .data(movement_stay,"movement_stay")
            .data(movement_w_m,"movement_w_m")
            .data(movement_m_e,"movement_m_e")
            .data(movement_w_e,"movement_w_e")

            .data(selectivities,"selectivities")

            .data(catches,"catches")
        ;
    }

	void read(void){
		// Read in any input files as usual
		Base::read();

		// Set default value and priors for recruitment deviations
		for(auto& recdev : recruits_deviations) recdev = Variate<Truncated<Normal>> {0,0,0.6,-3,3};

		// Set default value and priors for selectivities
		typedef Level<SelectivityKnot> knot;
		selectivities(knot(0)) = Variate<Uniform> {0.1,0,0.3}; // 20cm
		selectivities(knot(1)) = Variate<Uniform> {0.2,0,0.4}; // 30cm
		selectivities(knot(2)) = Variate<Uniform> {0.3,0,0.6}; // 40cm
		selectivities(knot(3)) = Variate<Uniform> {0.5,0,1.0}; // 50cm
		selectivities(knot(4)) = Variate<Uniform> {0.9,0,1.0}; // 60cm
		selectivities(knot(5)) = Variate<Uniform> {1.0,0,1.0}; // 70cm
		selectivities(knot(6)) = Variate<Uniform> {1.0,0,1.0}; // 80cm

		// Set missing catches to zero
		for(auto& catche : catches){
			if(not catche.valid()) catche = 0;
		}
	}

	/**
	 * Set model variables
	 */
	void set(Model& model, uint time) {
		uint year = IOSKJ::year(time);
		uint quarter = IOSKJ::quarter(time);
		
		// Bind invariant parameters
		if(time==0){
			model.recruits_unfished = std::exp(recruits_unfished);
			model.recruits_steepness = recruits_steepness;
			model.recruits_sd = recruits_sd;

			model.spawning(0) = spawning_0;
			model.spawning(1) = spawning_1;
			model.spawning(2) = spawning_2;
			model.spawning(3) = spawning_3;

			for(auto region : regions) model.recruits_regions(region) = recruits_regions;

			model.recruits_lengths_mean = recruits_lengths_mean;
			model.recruits_lengths_cv = recruits_lengths_cv;

			model.weight_length.a = weight_a;
			model.weight_length.b = weight_b;

			model.maturity_length.inflection = maturity_inflection;
			model.maturity_length.steepness = maturity_steepness;

			model.mortality_base = mortality_base;
			model.mortality_exponent = mortality_exponent;
			
			model.growth_rate = growth_rate;
			model.growth_assymptote = growth_assymptote;
			model.growth_sd = growth_sd;
			model.growth_cv = growth_cv;		

			for(auto region_from : region_froms){
				for(auto region : regions){
					if(region_from==Level<RegionFrom>(region)) model.movement_pars(region_from,region) = movement_stay;
					else if((region_from==W and region==M) or (region_from==M and region==W)) model.movement_pars(region_from,region) = movement_w_m;
					else if((region_from==M and region==E) or (region_from==E and region==M)) model.movement_pars(region_from,region) = movement_m_e;
					else if((region_from==W and region==E) or (region_from==E and region==W)) model.movement_pars(region_from,region) = movement_w_e;
					else throw std::runtime_error("Unhandled movement parameter");
				}
			}

			for(auto method : methods){
				for(auto knot : selectivity_knots){
					model.selectivity_values(method,knot) = selectivities(knot);
				}
			}
		}

		// Alternative parameterisation of recruitment variation depending on year..
		if(year<1990){
			// Deterministric recruitment
			model.recruits_variation_on = false;
			model.recruits_deviation = 1;
		}
		else if(year>=recdev_years.begin() and year<recdev_years.end()){
			// Stochastic recruitment defined by recruitment deviation parameters
			model.recruits_variation_on = false;
			model.recruits_deviation = std::exp(recruits_deviations(year));
		} else {
			// Stochastic recruitment defined by recruits_sd and recruits_auto as bound in time==0
			model.recruits_variation_on = true;
		}

		// Bind quarterly catch history to the model's catches
		if(year>=1950 and year<=2012){
			for(auto region : regions){
				for(auto method : methods){
					model.catches(region,method) = catches(year,quarter,region,method);
				}
			}
		}

		// Initialise in the first year
		if(time==0) model.initialise();
	}
}; // class IoskjParameters

} //namespace IOSKJ
