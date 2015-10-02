#pragma once

#include "model.hpp"
#include "variable.hpp"

namespace IOSKJ {

/**
 * Class for defining model parameters, their priors, transformations etc.
 * See the `set()` method which "binds" parameters defined here to model variables.
 */
class Parameters : public Structure<Parameters> {
public:
	
	/**
	 * Parameters of the stock-recruitment relationship
	 */
	Variable<Uniform> spawners_unfished;
	Variable<Uniform> recruits_steepness;
	Variable<Uniform> recruits_sd;

	/**
	 * Recruitment deviations
	 */
	Array<Variable<Normal>,RecdevYear> recruits_deviations;

	/**
	 * Proportion of mature fish spawning by quarter
	 */
	Variable<Uniform> spawning_0;
	Variable<Uniform> spawning_1;
	Variable<Uniform> spawning_2;
	Variable<Uniform> spawning_3;

	/**
	 * Proportion of recruits by region
	 * 
	 * These are normalised in the model initialisation
	 * so that they sum to one.
	 */
	Array<Variable<Uniform>,Region> recruits_regions;

	/**
	 * Variables of the distribution of the lengths of recruits
	 */
	Variable<Uniform> recruits_lengths_mean;
	Variable<Uniform> recruits_lengths_cv;

	/**
	 * Length-weight parameters
	 */
    Variable<Fixed> weight_a;
    Variable<Fixed> weight_b;

    /**
     * Maturity parameters
     */
    Variable<Uniform> maturity_inflection;
    Variable<Uniform> maturity_steepness;

    /**
     * Mortality parameters
     */
   	Variable<Uniform> mortality_base;
	Variable<Fixed> mortality_exponent;

    /**
     * Growth rate parameters
     */
	Variable<Fixed> growth_rate;
	Variable<Fixed> growth_assymptote;
	Variable<Fixed> growth_sd;
	Variable<Fixed> growth_cv;

    /**
     * Movements parameters
     *
     * There are 9 movement parameters (3 x 3 regions) but they are parameterised
     * into 4 priors. `movement_stay` defines a prior on the proportion of fish
     * that remain in an area.
     */
	Variable<Uniform> movement_stay;
	Variable<Uniform> movement_w_m;
	Variable<Uniform> movement_m_e;
	Variable<Uniform> movement_w_e;

	/**
	 * Selectivity parameters
	 */
	Array<Variable<Uniform>,Method,SelectivityKnot> selectivities;

	/**
	 * Catches by year, quarter, region and method
	 */
	Array<Variable<Fixed>,Year,Quarter,Region,Method> catches;

    /**
     * Reflection
     */
    template<class Mirror>
    void reflect(Mirror& mirror){
        mirror
            .data(spawners_unfished,"spawners_unfished")
            .data(recruits_steepness,"recruits_steepness")
            .data(recruits_sd,"recruits_sd")
            .data(recruits_deviations,"recruits_deviations")

            .data(spawning_0,"spawning_0")
            .data(spawning_1,"spawning_1")
            .data(spawning_2,"spawning_2")
            .data(spawning_3,"spawning_3")

            .data(recruits_regions,"recruits_regions")

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

    using Structure<Parameters>::read;

    void read(void){
    	Structure<Parameters>::read("parameters/input/parameters.json");
    	
    	recruits_regions.read("parameters/input/recruits_regions.tsv",true);
    	recruits_deviations.read("parameters/input/recruits_deviations.tsv",true);
    	selectivities.read("parameters/input/selectivities.tsv",true);
    	catches.read("parameters/input/catches.tsv",true);
		catches.each([](Variable<Fixed>& catche){
			if(catche.is_na()) catche = 0;
		});
    }

    using Structure<Parameters>::write;

    void write(void){
    	Structure<Parameters>::write("parameters/output/parameters.json");

    	recruits_regions.write("parameters/output/recruits_regions.tsv",true);
    	recruits_deviations.write("parameters/output/recruits_deviations.tsv",true);
    	selectivities.write("parameters/output/selectivities.tsv",true);
    	catches.write("parameters/output/catches.tsv",true);

    	values().write("parameters/output/values.tsv");
    }

	/**
	 * Set model variables
	 */
	void set(uint time, Model& model) const {
		uint year = IOSKJ::year(time);
		uint quarter = IOSKJ::quarter(time);
		
		// Bind invariant parameters
		if(time==0){
			model.biomass_spawners_unfished = spawners_unfished;
			model.recruits_steepness = recruits_steepness;
			model.recruits_sd = recruits_sd;

			model.spawning(0) = spawning_0;
			model.spawning(1) = spawning_1;
			model.spawning(2) = spawning_2;
			model.spawning(3) = spawning_3;

			for(auto region : regions) model.recruits_regions(region) = recruits_regions(region);

			model.recruits_lengths_mean = recruits_lengths_mean;
			model.recruits_lengths_cv = recruits_lengths_cv;

			model.weight_length_a = weight_a;
			model.weight_length_b = weight_b;

			model.maturity_length_inflection = maturity_inflection;
			model.maturity_length_steepness = maturity_steepness;

			model.mortality_base = mortality_base;
			model.mortality_exponent = mortality_exponent;
			
			model.growth_rate = growth_rate;
			model.growth_assymptote = growth_assymptote;
			model.growth_sd = growth_sd;
			model.growth_cv = growth_cv;		

			for(auto region_from : region_froms){
				for(auto region : regions){
					if(region_from==Level<RegionFrom>(region)) model.movement_pars(region_from,region) = movement_stay;
					// TODO else if((region_from==W and region==M) or (region_from==M and region==W)) model.movement_pars(region_from,region) = movement_w_m;
					// TODO else if((region_from==M and region==E) or (region_from==E and region==M)) model.movement_pars(region_from,region) = movement_m_e;
					// TODO else if((region_from==W and region==E) or (region_from==E and region==W)) model.movement_pars(region_from,region) = movement_w_e;
					else throw std::runtime_error("Unhandled movement parameter");
				}
			}

			for(auto method : methods){
				for(auto knot : selectivity_knots){
					model.selectivity_values(method,knot) = selectivities(method,knot);
				}
			}
		}

		// Alternative parameterization of recruitment variation depending on year..
		if(year<1985){
			// Deterministic recruitment
			model.recruits_variation_on = false;
			model.recruits_deviation = 1;
		}
		else if(year>=recdev_years.begin() and year<recdev_years.end()){
			// Stochastic recruitment defined by recruitment deviation parameters
			model.recruits_variation_on = false;
			model.recruits_deviation = std::exp(recruits_deviations(year));
		} else {
			// Stochastic recruitment defined by recruits_sd and recruits_auto
			model.recruits_variation_on = true;
		}

		// Bind quarterly catch history to the model's catches
		if(year>=1950 and year<=2012){
			model.exploit = model.exploit_catch;
			for(auto region : regions){
				for(auto method : methods){
					model.catches(region,method) = catches(year,quarter,region,method);
				}
			}
		}

		// Set effort for all regions and methods
		// at a nominal 100 units
		if(year<2004) model.effort = 0;
		if(year>=2004 and year<=2013) model.effort = 100;

		// Initialise in the first year
		if(time==0) model.initialise();
	}

	/**
	 * A base Mirror class used for mirroring only variables that are not fixed
	 */
	template<class Derived>
	struct Variabler : Mirrors::Mirror<Derived> {
		using Polymorph<Derived>::derived;

		template<class Distribution, class... Dimensions>
		Derived& data(Array<Variable<Distribution>,Dimensions...>& array, const std::string& name){
			// Recurse into arrays of variables
			array.reflect(derived());
			return derived();
		}

		template<class... Dimensions>
		Derived& data(Array<Variable<Fixed>,Dimensions...>& array, const std::string& name){
			// Ignore arrays of fixed variables
			return derived();
		}

		Derived& data(Variable<Fixed>& variable, const std::string& name){
			// Ignore fixed variables
			return derived();
		}
	};

	/**
	 * Randomise the values of variables
	 */
	void randomise(void){
		Randomiser().mirror(*this);
	}
	struct Randomiser : Variabler<Randomiser> {
		using Variabler<Randomiser>::data;

		template<class Distribution>
		Randomiser& data(Variable<Distribution>& variable, const std::string& name){
			variable.value = variable.random();
			return *this;
		}
	};

	/**
	 * Get the values of variables
	 */
	Frame values(void){
		return Values().mirror(*this).values;
	}
	struct Values : Variabler<Values> {
		using Variabler<Values>::data;
		Frame values;
		std::string prefix;

		Values():values(1){}

		template<class Distribution, class... Dimensions>
		Values& data(Array<Variable<Distribution>,Dimensions...>& array, const std::string& name){
			prefix = name;
			array.reflect(*this);
			prefix = "";
			return *this;
		}

		template<class Distribution>
		Values& data(Variable<Distribution>& variable, const std::string& name){
			values.add(prefix+name+".value",variable.value);
			return *this;
		}
	};

}; // class Parameters

} //namespace IOSKJ
