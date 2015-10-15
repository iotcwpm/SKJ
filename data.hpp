#pragma once

#include "model.hpp"
#include "variable.hpp"

namespace IOSKJ {

/**
 * Data against which the model is conditioned
 * 
 * See the `get()` method which "gets" model variables corresponding to data at specific times.
 */
class Data : public Structure<Data> {
public:

	/**
	 * Maldive pole and line quarterly CPUE
	 */
	Array<Variable<Lognormal>,DataYear,Quarter> m_pl_cpue;

	/**
	 * West purse seine annual CPUE
	 */
	Array<Variable<Lognormal>,DataYear> w_ps_cpue;

	/**
	 * Z-estimates
	 */
	Array<Variable<Normal>,DataYear,Quarter,ZSize> z_ests;

	/**
	 * Size frequencies
	 */
	typedef Variable<FournierRobustifiedMultivariateNormal> SizeFreqVariable;
	Array<SizeFreqVariable,DataYear,Quarter,Region,Method,Size> size_freqs;

	/**
	 * Count of years in which estimated Z>0.9
	 */
	double exp_rate_high;

	/**
	 * Log-likelihoods for each data sets
	 */
	double m_pl_cpue_ll;
	double w_ps_cpue_ll;
	double z_ests_ll;
	double size_freqs_ll;
	double exp_rate_high_ll;

    /**
     * Reflection
     */
    template<class Mirror>
    void reflect(Mirror& mirror){
        mirror
            .data(m_pl_cpue,"m_pl_cpue")
            .data(w_ps_cpue,"w_ps_cpue")
            .data(z_ests,"z_ests")
            .data(size_freqs,"size_freqs")
        ;
    }

    void read(void){
    	m_pl_cpue.read("data/input/m_pl_cpue.tsv",true);
    	w_ps_cpue.read("data/input/w_ps_cpue.tsv",true);
    	size_freqs.read("data/input/size_freqs.tsv",true);
    	z_ests.read("data/input/z_ests.tsv",true);
    }

    void write(void){
    	m_pl_cpue.write("data/output/m_pl_cpue.tsv",true);
    	w_ps_cpue.write("data/output/w_ps_cpue.tsv",true);
    	z_ests.write("data/output/z_ests.tsv",true);
    	size_freqs.write("data/output/size_freqs.tsv",
    		{"value","proportion","size","sd"},
    		[](std::ostream& stream, const SizeFreqVariable& var){
    			stream<<var<<"\t"<<var.proportion<<"\t"<<var.size<<"\t"<<var.sd();
    		}
    	);
    }

	/**
	 * Get model variables corresponding to data at a particular time
	 *
	 * For each data set, predictions are generated outside of the range of observed
	 * data. This is for diagnosis and future proofing (when more observed data become available
	 * and are added to data files the model will already be set up to fit that it). 
	 * There will be a small computational cost to this.
	 */
	void get(uint time, const Model& model){
		uint year = IOSKJ::year(time);
		uint quarter = IOSKJ::quarter(time);
		
		// Maldive PL quarterly CPUE
		if(year>=2000 and year<=2014){
			// Just get M/PL vulnerable biomass
			m_pl_cpue(year,quarter) = model.biomass_vulnerable(MA,PL) * model.m_pl_quarter(quarter);	
			
			// At end, scale expected by geometric mean over period 2004-2012
			if(year==2014 and quarter==3){
				GeometricMean geomean;
				for(uint year=2004;year<=2012;year++){
					for(uint quarter=0;quarter<4;quarter++){
						geomean.append(m_pl_cpue(year,quarter));
					}
				}
				double scaler = 1/geomean.result();
				for(auto& fit : m_pl_cpue) fit *= scaler;
			}
		}

		// West PS annual CPUE
		if(year>=1985 and year<=2014){
			// Currently take a mean of vulnerable biomass over all quarters in the year...
			static Array<double,Quarter> cpue_quarters;
			// ... get this quarter's CPUE and save it
			cpue_quarters(quarter) = model.biomass_vulnerable(SW,PS) + model.biomass_vulnerable(NW,PS);
			// ... if this is the last quarter then take the geometric mean
			if(quarter==3){
				w_ps_cpue(year) = geomean(cpue_quarters);
			}	

			// At end, scale expected by geometric mean over period 1991-2010
			if(year==2014 and quarter==3){
				GeometricMean geomean;
				for(uint year=1991;year<=2010;year++){
					geomean.append(w_ps_cpue(year,quarter));
				}
				double scaler = 1/geomean.result();
				for(auto& fit : w_ps_cpue) fit *= scaler;
			}	
		}

		// Size frequencies by region and method
		if(year>=1982 and year<=2014){
			// Generate expected size frequencies for each method in each 
			// region regardless of whether there is observed data or not
			for(auto region : regions){
				for(auto method : methods){
					Array<double,Size> composition = 0;
					// Calculate selected numbers by size accumulated over ages
					for(auto size : sizes){
						for(auto age : ages){
							composition(size) += model.numbers(region,age) * 
												 model.age_size(age,size) *
												 model.selectivity_size(method,size);
						}
					}
					// Proportionalise
					composition /= sum(composition);
					// Store
					for(auto size : sizes) size_freqs(year,quarter,region,method,size) = composition(size);
				}
			}
		}

		// Size based Z-estimates for NW region from tagging
		if(year>=2005 and year<=2014){
			// Generate expected values of Z for each size bin
			// Expected values of Z are calculated by combining natural mortality and 
			// fishing mortality rates
			for(auto z_size : z_sizes){
				// Model size classes are 2mm wide, so for each of the 5mm wide Z-estimate bins there are three model
				// size classes to average over e.g. 
				//   45-50 Z-estimate size bin ~ 45,47,49 model size class mid points ~ ([44,46,48])/2 ~ 22,23,24 size dimension levels
				// Calculate the mean Z for the model size classes in each Z-estimate size bin
				double z = 0;
				uint z_lower = 45+z_size.index()*5;
				uint size_class = (z_lower-1)/2;
				for(uint size=size_class;size<size_class+3;size++){
					// Calculate a weighted overall survival across age classes for the size
					double numerator = 0;
					double denominator = 0;
					for(auto age : ages){
						// Expected number in this size bin
						double number = model.numbers(NW,age) * model.age_size(age,size);
						// Survival for this age
						double survival = model.survival(age) * model.escapement(NW,age);
						// Add to average
						numerator += survival*number;
						denominator += number;
					}
					// Calculate weighted mean
					double survival = numerator/denominator;
					// Capture cases where survuval is estimated to be zero to prevent overflow
					if(survival>0) z += -log(survival);
					else z += -log(0.000001);
				}
				z /= 3;
				// Store
				z_ests(year,quarter,z_size) = z;
			}
		}

		// Check number of region/gear exploitation rates that are above 90%;
		if(year==year_min) exp_rate_high = 0;
		for(auto& er : model.exploitation_rate){
			if(er>0.9) exp_rate_high++;
		}
	}

	double loglike(void){
		m_pl_cpue_ll = 0;
		for(auto& item : m_pl_cpue) m_pl_cpue_ll += item.loglike();

		w_ps_cpue_ll = 0;
		for(auto& item : w_ps_cpue) w_ps_cpue_ll += item.loglike();

		z_ests_ll = 0;
		for(auto& item : z_ests) z_ests_ll += item.loglike();

		FournierRobustifiedMultivariateNormal::max_size = 30;
		size_freqs_ll = 0;
		for(auto& item : size_freqs) size_freqs_ll += item.loglike();

		exp_rate_high_ll = -exp_rate_high;

    	return m_pl_cpue_ll + w_ps_cpue_ll + z_ests_ll + size_freqs_ll + exp_rate_high_ll;
    }

}; // class Data

} // namespace IOSKJ
