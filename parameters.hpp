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
	Variable<Uniform> recruits_unfished;
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
	 * Prior same for all regions. These are normalised in the model initialisation
	 * so that they sum to one.
	 */
	Variable<Uniform> recruits_regions;

	/**
	 * Variables of the distribution of the lengths of recruits
	 */
	Variable<Uniform> recruits_lengths_mean ;
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
	Array<Variable<Uniform>,SelectivityKnot> selectivities;

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
            .data(recruits_unfished,"recruits_unfished")
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

    void read(void){
    	Structure<Parameters>::read("parameters/input/parameters.cila");
    	recruits_deviations.read("parameters/input/recruits_deviations.tsv",true);
    	selectivities.read("parameters/input/selectivities.tsv",true);
    	catches.read("parameters/input/catches.tsv",true);
		catches.each([](Variable<Fixed>& catche){
			if(catche.is_na()) catche = 0;
		});
    }

    void write(void){
    	Structure<Parameters>::write("parameters/output/parameters.cila");
    	recruits_deviations.write("parameters/output/recruits_deviations.tsv",true);
    	selectivities.write("parameters/output/selectivities.tsv",true);
    	catches.write("parameters/output/catches.tsv",true);
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

	/**
	 * A base Mirror class used for mirroring only variables that are not fixed
	 */
	template<class Derived>
	struct Variabler : Mirrors::Mirror<Derived> {
		using Polymorph<Derived>::derived;

		Derived& mirror(Parameters& parameters){
			parameters.reflect(derived());
			return derived();
		}

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
	 * Get a list of parameter names
	 *
	 * This method differs from `Structure::labels` in that it does not
	 * add the inner names of variables, just the variable names themselves,
	 * and ignores `Variable<Fixed>`
	 */
	std::vector<std::string> labels(void) {
		return Labeller().mirror(*this).labels;
	}
	struct Labeller : Variabler<Labeller> {
		using Variabler<Labeller>::data;
		std::string prefix;
		std::vector<std::string> labels;

		template<class Distribution, class... Dimensions>
		Labeller& data(Array<Variable<Distribution>,Dimensions...>& array, const std::string& name){
			prefix = name;
			array.reflect(*this);
			prefix = "";
			return *this;
		}

		template<class Distribution>
		Labeller& data(Variable<Distribution>& variable, const std::string& name){
			labels.push_back(prefix+name);
			return *this;
		}
	};

	/**
	 * Get a sample from the parameter prior distributions
	 *
	 * @parameter number Number of samples to get
	 */
	Frame<> sample(unsigned int number=1){
		auto labs = labels();
		unsigned int cols = labs.size();
		Frame<> samples(number,labs);
		for(unsigned int row=0;row<number;row++){
			auto sample = Sampler().mirror(*this).sample;
			for(unsigned int col=0;col<cols;col++){
				samples(row,col) = sample[col];
			}
		}
		return samples;
	}
	struct Sampler : Variabler<Sampler> {
		using Variabler<Sampler>::data;
		std::vector<double> sample;

		template<class Distribution>
		Sampler& data(Variable<Distribution>& variable, const std::string& name){
			sample.push_back(variable.random());
			return *this;
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
			variable = variable.random();
			return *this;
		}
	};

	/**
	 * Get the values of variables
	 */
	std::vector<double> values(void){
		return Values().mirror(*this).values;
	}
	struct Values : Variabler<Values> {
		using Variabler<Values>::data;
		std::vector<double> values;

		template<class Distribution>
		Values& data(Variable<Distribution>& variable, const std::string& name){
			values.push_back(variable);
			return *this;
		}
	};

}; // class Parameters

} //namespace IOSKJ
