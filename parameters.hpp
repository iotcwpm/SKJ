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
	 * Overall unfished spawning biomass
	 */
	Variable<Uniform> spawners_unfished;

	/**
	 * Relative recruitment in each of the smaller regions
	 * 
	 * These are relative to the unfished spawning biomass
	 * going to WE region. All proportions are 
	 * then normalised in the model initialisation
	 * so that they sum to one.
	 */
	Variable<Uniform> spawners_ma;
	Variable<Uniform> spawners_ea;

	/**
	 * Special distribution for steepness prior
	 * based on beta. Note that a lower bound
	 * of 0.6 is used
	 */
	struct SteepnessBeta : Beta {
		double minimum(void) const {
			return 0.6;
		}
		double maximum(void) const {
			return 1;
		}
		double random(void) const {
			double trial = 0;
			while(trial<0.6){
				trial = (Beta::random()+0.25)/1.25;
			}
			return trial;
		}
		double loglike(const double& steepness) const {
			if(steepness<0.6) return -INFINITY;
			else return std::log(Beta::pdf(steepness*1.25-0.25));
		}
	};
	Variable<SteepnessBeta> recruits_steepness;

	/**
	 * Standard deviation of recruitment deviations
	 */
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
	 * Length-weight parameters
	 */
    Variable<Fixed> weight_a;
    Variable<Fixed> weight_b;

    /**
     * Maturity parameters
     */
    Variable<TruncatedNormal> maturity_inflection;
    Variable<TruncatedNormal> maturity_steepness;

    /**
     * Mortality parameters
     */
   	Variable<Uniform> mortality_base;
   	Variable<Uniform> mortality_base_age;
	Variable<Uniform> mortality_exponent_1;
	Variable<Uniform> mortality_exponent_2;

    /**
     * Growth rate parameters
     */
	Variable<Fixed> growth_rate_1;//!
	Variable<Fixed> growth_rate_2;//!
	Variable<Fixed> growth_assymptote;//!
	Variable<Fixed> growth_stanza_inflection;//!
	Variable<Fixed> growth_stanza_steepness;//!
	Variable<Fixed> growth_age_0;
	Variable<Fixed> growth_cv_0;//!
	Variable<Fixed> growth_cv_old;//!

    /**
     * Movements parameters
     */
	Variable<Uniform> movement_we_ma;
	Variable<Uniform> movement_we_ea;
	Variable<Uniform> movement_ma_ea;
    Variable<Fixed> movement_length_inflection;//!
    Variable<Fixed> movement_length_steepness;//!

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
            .data(spawners_ma,"spawners_ma")
            .data(spawners_ea,"spawners_ea")

            .data(recruits_steepness,"recruits_steepness")
            .data(recruits_sd,"recruits_sd")
            .data(recruits_deviations,"recruits_deviations")

            .data(spawning_0,"spawning_0")
            .data(spawning_1,"spawning_1")
            .data(spawning_2,"spawning_2")
            .data(spawning_3,"spawning_3")

            .data(weight_a,"weight_a")
            .data(weight_b,"weight_b")

            .data(maturity_inflection,"maturity_inflection")
            .data(maturity_steepness,"maturity_steepness")

            .data(mortality_base,"mortality_base")
            .data(mortality_base_age,"mortality_base_age")
            .data(mortality_exponent_1,"mortality_exponent_1")
            .data(mortality_exponent_2,"mortality_exponent_2")

			.data(growth_rate_1,"growth_rate_1")
			.data(growth_rate_2,"growth_rate_2")
			.data(growth_assymptote,"growth_assymptote")
			.data(growth_stanza_inflection,"growth_stanza_inflection")
			.data(growth_stanza_steepness,"growth_stanza_steepness")
			.data(growth_age_0,"growth_age_0")
			.data(growth_cv_0,"growth_cv_0")
			.data(growth_cv_old,"growth_cv_old")

            .data(movement_we_ma,"movement_we_ma")
            .data(movement_we_ea,"movement_we_ea")
            .data(movement_ma_ea,"movement_ma_ea")

            .data(movement_length_inflection,"movement_length_inflection")
            .data(movement_length_steepness,"movement_length_steepness")

            .data(selectivities,"selectivities")

            .data(catches,"catches")
        ;
    }

    using Structure<Parameters>::read;

    void read(void){
    	Structure<Parameters>::read("parameters/input/parameters.json");
    	
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

    	recruits_deviations.write("parameters/output/recruits_deviations.tsv",true);
    	selectivities.write("parameters/output/selectivities.tsv",true);
    	catches.write("parameters/output/catches.tsv",true);

    	values().write("parameters/output/values.tsv");
    }

	/**
	 * Set model variables
	 *
	 * @parameters catches_apply Turn off application of catches (e.g. in hindcasts of procedures)
	 */
	void set(uint time, Model& model, bool catches_apply = true) const {
		uint year = IOSKJ::year(time);
		uint quarter = IOSKJ::quarter(time);
		
		// Bind time-invariant parameters to model attributes
		if(time==0){
			// Stock - recruitment
			// Apportion `spawners_unfished` to regions
			Array<double,Region> props = {
				double(1.0),
				double(spawners_ma),
				double(spawners_ea)
			};
			props /= sum(props);
			for(auto region : regions) model.biomass_spawners_unfished(region) = spawners_unfished * props(region);

			model.recruits_steepness = recruits_steepness;
			model.recruits_sd = recruits_sd;

			// Proportion of mature fish spawning in each quarter
			model.spawning(0) = spawning_0;
			model.spawning(1) = spawning_1;
			model.spawning(2) = spawning_2;
			model.spawning(3) = spawning_3;

			// Length-weight relationship
			model.weight_length_a = weight_a;
			model.weight_length_b = weight_b;

			// Maturity curve
			model.maturity_length_inflection = maturity_inflection;
			model.maturity_length_steepness = maturity_steepness;

			// Mortality-at-age curve
			model.mortality_base = mortality_base;
			model.mortality_base_age = mortality_base_age;
			model.mortality_exponent_1 = mortality_exponent_1;
			model.mortality_exponent_2 = mortality_exponent_2;
			
			// Growth curve
			model.growth_rate_1 = growth_rate_1;
			model.growth_rate_2 = growth_rate_2;
			model.growth_assymptote = growth_assymptote;
			model.growth_stanza_inflection = growth_stanza_inflection;
			model.growth_stanza_steepness = growth_stanza_steepness;
			model.growth_cv_0 = growth_cv_0;
			model.growth_cv_old = growth_cv_old;

			// Movement
			// Note that in the model.intialise function these
			// proportional are restricted so that they do not sum to greater
			// than one.
			model.movement_region(WE,WE) = 1-movement_we_ma-movement_we_ea;
			model.movement_region(WE,MA) = movement_we_ma;
			model.movement_region(WE,EA) = movement_we_ea;

			model.movement_region(MA,WE) = movement_we_ma;
			model.movement_region(MA,MA) = 1-movement_we_ma-movement_ma_ea;
			model.movement_region(MA,EA) = movement_ma_ea;

			model.movement_region(EA,WE) = movement_we_ea;
			model.movement_region(EA,MA) = movement_ma_ea;
			model.movement_region(EA,EA) = 1-movement_ma_ea-movement_we_ea;

			model.movement_length_inflection = movement_length_inflection;
			model.movement_length_steepness = movement_length_steepness;

			// Selectivity
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
		}
		else if(year>=recdev_years.end() and time<=time_now){
			// Deterministic recruitment otherwise get
			// different fits from same parameter sets
			// during conditioning
			model.recruits_variation_on = false;
			model.recruits_deviation = 1;
		}
		else {
			// Stochastic recruitment defined by recruits_sd and recruits_auto
			model.recruits_variation_on = true;
		}

		// Bind quarterly catch history to the model's catches
		if(catches_apply and year>=1950 and year<=2014){
			model.exploit = model.exploit_catch;
			for(auto region : regions){
				for(auto method : methods){
					model.catches(region,method) = catches(
						// Carry over 2012 catch pattern in following year
						// until new catch estimates are available
						(year <=2012) ? year : 2012,
						quarter,
						region,
						method
					);
				}
			}
		}

		// Set effort for all regions and methods
		// at a nominal 100 units
		if(year<2004) model.effort = 0;
		if(year>=2004 and year<=2014) model.effort = 100;

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
	Parameters& randomise(void){
		Randomiser().mirror(*this);
		return *this;
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
	 * Bounce values of parameters off bounds to ensure that they are within bounds
	 * but not bunched up against them
	 */
	Parameters& bounce(void){
		Restrictor().mirror(*this);
		return *this;
	}
	struct Restrictor : Variabler<Restrictor> {
		using Variabler<Restrictor>::data;

		template<class Distribution>
		Restrictor& data(Variable<Distribution>& variable, const std::string& name){
			if(variable.value>variable.maximum()){
				variable.value = std::max(variable.maximum()-(variable.value-variable.maximum()),variable.minimum());
			}
			else if(variable.value<variable.minimum()){
				variable.value = std::min(variable.minimum()+(variable.minimum()-variable.value),variable.maximum());
			}
			return *this;
		}
	};

	/**
	 * Calculate prior likelihoods for parameters
	 */
	double loglike(void){
		return Logliker().mirror(*this).loglike;
	}
	struct Logliker : Variabler<Logliker> {
		using Variabler<Logliker>::data;
		double loglike = 0;

		template<class Distribution>
		Logliker& data(Variable<Distribution>& variable, const std::string& name){
			loglike += variable.loglike();
			return *this;
		}
	};

	/**
	 * Get the names of variables
	 */
	std::vector<std::string> names(void){
		return Names().mirror(*this).names;
	}
	struct Names : Variabler<Names> {
		using Variabler<Names>::data;
		std::vector<std::string> names;
		std::string prefix;

		template<class Distribution, class... Dimensions>
		Names& data(Array<Variable<Distribution>,Dimensions...>& array, const std::string& name){
			prefix = name;
			array.reflect(*this);
			prefix = "";
			return *this;
		}

		template<class Distribution>
		Names& data(Variable<Distribution>& variable, const std::string& name){
			names.push_back(prefix+name+".value");
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

	/**
	 * Get the values of variables as a vector
	 */
	std::vector<double> vector(void){
		return VectorGetter().mirror(*this).values;
	}
	struct VectorGetter : Variabler<VectorGetter> {
		using Variabler<VectorGetter>::data;
		std::vector<double> values;

		template<class Distribution>
		VectorGetter& data(Variable<Distribution>& variable, const std::string& name){
			values.push_back(variable.value);
			return *this;
		}
	};

	/**
	 * Set the values of variables from a vector
	 */
	void vector(const std::vector<double>& vector){
		VectorSetter(vector).mirror(*this);
	}
	struct VectorSetter : Variabler<VectorSetter> {
		using Variabler<VectorSetter>::data;
		std::vector<double> values;
		int index;

		VectorSetter(const std::vector<double>& vector){
			values = vector;
			index = 0;
		}

		template<class Distribution>
		VectorSetter& data(Variable<Distribution>& variable, const std::string& name){
			variable.value = values[index];
			index++;
			return *this;
		}
	};

}; // class Parameters

} //namespace IOSKJ
