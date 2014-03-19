#pragma once

#include <boost/format.hpp>
using boost::format;

#include <fsl/estimation/parameters.hpp>
#include <fsl/math/probability/uniform.hpp>
#include <fsl/math/probability/normal.hpp>
#include <fsl/math/probability/truncated.hpp>

namespace IOSKJ {

/**
 * Class for defining model parameters, their priors, transformations etc.
 * See the `bind()` method which "binds" parameters defined here to model variables.
 */
class Parameters : public Fsl::Estimation::Parameters<Parameters,Model> {
public:

	// Define some aliases for convienience
	
	template<typename... Types> 
	using Parameter = Fsl::Estimation::Parameter<Types...>;

	using Uniform = Fsl::Math::Probability::Uniform;
	using Normal = Fsl::Math::Probability::Normal;
	template<typename Type>
	using Truncated = Fsl::Math::Probability::Truncated<Type>;

	using Fixed = Fsl::Estimation::Priors::Fixed;
	using Log = Fsl::Estimation::Links::Log;

	/**
	 * Define begin and end time for parameter binding
	 * The `bind` method is called over this range to generate a parameter set
	 */
	uint begin(void) const {
		return 0;
	}
	uint end(void) const {
		return time(2014,0);
	}
	
	/**
	 * Parameters of the stock-recruitment relationship
	 */
	Parameter<Uniform,Log> recruits_unfished = {"recruits_unfished",{10,30}};
	Parameter<Uniform> recruits_steepness = {"recruits_steepness",{0.7,1.0}};
	Parameter<Uniform> recruits_sd = {"recruits_sd",{0.4,0.8}};

	/**
	 * Recruitment deviations
	 */
	Parameter<Truncated<Normal>,Log> recruits_deviation = {"recruits_deviation",{0,0.6,-3,3}};

	/**
	 * Proportion of mature fish spawning by quarter
	 */
	Parameter<Uniform> spawning_0 = {"spawning_0",{0.7,1}};
	Parameter<Uniform> spawning_1 = {"spawning_1",{0.3,0.7}};
	Parameter<Uniform> spawning_2 = {"spawning_2",{0.7,1}};
	Parameter<Uniform> spawning_3 = {"spawning_3",{0.3,0.7}};

	/**
	 * Proportion of recruits by region
	 * Prior same for all regions. These are normalised in the model initialisation
	 * so that they sum to one.
	 */
	Parameter<Uniform> recruits_regions = {"recruits_regions",{0.1,0.9}};

	/**
	 * Parameters of the distribution of the size of recruits
	 */
	Parameter<Uniform> recruits_size_mean = {"recruits_size_mean",{1,10}};
	Parameter<Uniform> recruits_size_cv = {"recruits_size_cv",{0.1,0.2}};

	/**
	 * Length-weight parameters
	 */
    Parameter<Uniform> weight_a = {"weight_a",{5.32e-6,5.32001e-6}};
    Parameter<Uniform> weight_b = {"weight_b",{3.35,3.35001}};

    /**
     * Maturity parameters
     */
    Parameter<Uniform> maturity_inflection = {"maturity_inflection",{40,45}};
    Parameter<Uniform> maturity_steepness = {"maturity_steepness",{4,6}};

    /**
     * Mortality parameters
     */
   	Parameter<Uniform> mortality = {"mortality",{0.5,0.9}};
	Parameter<Fixed> mortality_weight_exponent = {"mortality_weight_exponent",-0.29};
	Parameter<Fixed> mortality_max = {"mortality_max",-std::log(0.01)};

    /**
     * Growth rate parameters
     */
	Parameter<Fixed> growth_rate = {"growth_rate",0.3};
	Parameter<Fixed> growth_assymptote = {"growth_assymptote",75};
	Parameter<Fixed> growth_sd = {"growth_sd",1};
	Parameter<Fixed> growth_cv = {"growth_cv",0.2};

    /**
     * Movements parameters
     *
     * There are 9 movement parameters (3 x 3 regions) but they are parameterised
     * into 4 priors. `movement_stay` defines a prior on the proportion of fish
     * that remain in an area
     */
	Parameter<Uniform> movement_stay = {"movement_stay",{0.6,1.0}};
	Parameter<Uniform> movement_w_m = {"movement_w_m",{0.0,0.4}};
	Parameter<Uniform> movement_m_e = {"movement_m_e",{0.0,0.4}};
	Parameter<Uniform> movement_w_e = {"movement_w_e",{0.0,0.2}};

	/**
	 * Selectivity parameters
	 *
	 * These are likely to be well determined from the data and so a U(0,1) prior
	 * is used for all
	 */
	Parameter<Uniform> selectivity = {"selectivity",{0,1}};

	/**
	 * Catches by time, region and method
	 */
	Grid<
		Parameter<Fixed>,
		Time,Region,Method
	> catches;

	/**
	 * Bind parameters to model variables
	 */
	template<class Binder>
	void bind(Binder& binder, Model& model, uint time) {
		uint year = IOSKJ::year(time);
		uint quarter = IOSKJ::quarter(time);
		
		// Bind invariant parameters
		if(time==0){
			binder(recruits_unfished, model.recruits_unfished);
			binder(recruits_steepness, model.recruits_steepness);
			binder(recruits_sd, model.recruits_sd);

			binder(spawning_0, model.spawning(0));
			binder(spawning_1, model.spawning(1));
			binder(spawning_2, model.spawning(2));
			binder(spawning_3, model.spawning(3));

			for(auto region : regions) {
				binder(
					recruits_regions,
					model.recruits_regions(region),
					str(format("recruits_regions_%s")%region)
				);
			}

			binder(recruits_size_mean, model.recruits_size_mean);
			binder(recruits_size_cv, model.recruits_size_cv);

			binder(weight_a, model.weight_a);
			binder(weight_b, model.weight_b);

			binder(maturity_inflection, model.maturity_inflection);
			binder(maturity_steepness, model.maturity_steepness);

			binder(mortality,model.mortality);
			binder(mortality_weight_exponent,model.mortality_weight_exponent);
			binder(mortality_max,model.mortality_max);

			binder(growth_rate, model.growth_rate);
			binder(growth_assymptote, model.growth_assymptote);
			binder(growth_sd, model.growth_sd);
			binder(growth_cv, model.growth_cv);		

			for(auto region_from : region_froms){
				for(auto region : regions){
					auto label = str(format("movement_%s_%s")%region_from%region);
					if(region_from==region) binder(movement_stay, model.movement_pars(region_from,region), label);
					else if((region_from==W and region==M) or (region_from==M and region==W)) binder(movement_w_m, model.movement_pars(region_from,region), label);
					else if((region_from==M and region==E) or (region_from==E and region==M)) binder(movement_m_e, model.movement_pars(region_from,region), label);
					else if((region_from==W and region==E) or (region_from==E and region==W)) binder(movement_w_e, model.movement_pars(region_from,region), label);
					else throw std::runtime_error("Unhandled movement parameter:"+label);
				}
			}

			for(auto method : methods){
				for(auto knot : selectivity_knots){
					binder(selectivity, model.selectivity_points(method,knot),str(format("selectivity_%s_%s")%method%knot));
				}
			}

			// If the binder is a "setter" then need to initialise the model
			if(binder.setter){
				model.initialise();
			}
		}

		// Alternative parameterisation of recruitment variation epending on year..
		if(year<1990){
			// Deterministric recruitment
			model.recruits_variation_on = false;
			model.recruits_deviation = 1;
		}
		else if(quarter==0 and year>=1990 and year<=2012){
			// Stochastic recruitment defined by recruitment deviation parameters
			// Bind recruitment deviations in the first quarter (only one deviation per year) where information available
			model.recruits_variation_on = false;
			binder(
				recruits_deviation,
				model.recruits_deviation,
				str(format("recruits_deviation_%s")%year)
			);
		} else {
			// Stochastic recruitment defined by recruits_sd and recruits_auto as bound in time==0
			model.recruits_variation_on = true;
		}

		// Bind quarterly catch history to model catches
		for(auto region : regions){
			for(auto method : methods){
				binder(
					catches(time,region,method),
					model.catches(region,method),
					str(format("catch_%s_%s_%s_%s")%year%quarter%region%method)
				);
			}
		}
	}

};

};