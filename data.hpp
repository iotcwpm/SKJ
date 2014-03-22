#pragma once

#include "model.hpp"

namespace IOSKJ {

/**
 * Class for defining data against which the model is conditioned
 * See the `get()` method which "gets" model variables corresponding to data points at specific times.
 */
class Data : public DataGroup<Data,Model> {
public:

	Fits<Lognormal,DataYear,Quarter> maldive_pl_cpue = 0.2;

	Fits<Lognormal,DataYear> west_ps_cpue = 0.3;

	Fits<Normal,DataYear,Quarter,Region,Method,Size> size_freqs = 0.01;

	Fits<Normal,DataYear,Quarter,ZSize> z_ests = 0.05;

	/**
	 * Get model variables corresponding to data at a particular time
	 *
	 * For each data set, predictions are generated outside of the range of observed
	 * data. This is for diagnosis and future proofing (when more observed data become available
	 * and are added to data files the model will already be set up to fit that it). 
	 * There will be a small computational cost to this.
	 */
	void get(const Model& model,uint time){
		uint year = IOSKJ::year(time);
		uint quarter = IOSKJ::quarter(time);
		
		// Maldive PL quarterly CPUE
		if(year>=2000 and year<=2013){
			// Just get M/PL vulnerable biomass
			maldive_pl_cpue(year,quarter).expected = model.biomass_vulnerable(M,PL);	
			
			// At end, scale expected by geometric mean over period 2004-2012
			if(year==2013 and quarter==3){
				GeometricMean geomean;
				for(uint year=2004;year<=2012;year++){
					for(uint quarter=0;quarter<4;quarter++){
						geomean.append(maldive_pl_cpue(year,quarter).expected);
					}
				}
				double scaler = 1/geomean.result();
				for(auto& fit : maldive_pl_cpue) fit.expected *= scaler;
			}
		}

		// West PS annual CPUE
		if(year>=1985 and year<=2013){
			// Currently take a mean of vulnerable biomass over all quarters in the year...
			static Grid<double,Quarter> cpue_quarters;
			// ... get this quarter's CPUE and save it
			cpue_quarters(quarter) = model.biomass_vulnerable(W,PS);
			// ... if this is the last quarter then take the geometric mean
			if(quarter==3){
				west_ps_cpue(year).expected = geomean(cpue_quarters);
			}	

			// At end, scale expected by geometric mean over period 1991-2010
			if(year==2013 and quarter==3){
				GeometricMean geomean;
				for(uint year=1991;year<=2010;year++){
					geomean.append(west_ps_cpue(year,quarter).expected);
				}
				double scaler = 1/geomean.result();
				for(auto& fit : west_ps_cpue) fit.expected *= scaler;
			}	
		}

		// Size frequencies by region and method
		if(year>=1982 and year<=2013){
			// Generate expected size frequencies for each method in each 
			// region regardless of whether there is observed data or not
			for(auto region : regions){
				for(auto method : methods){
					Grid<double,Size> composition = 0;
					// Calculate selected numbers by size accumulated over ages
					for(auto size : sizes){
						for(auto age : ages){
							composition(size) += model.numbers(region,age,size) * model.selectivities(method,size);
						}
					}
					// Proportionalise
					composition /= sum(composition);
					// Store
					for(auto size : sizes) size_freqs(year,quarter,region,method,size).expected = composition(size);
				}
			}
		}

		// Z-estimates for W region
		if(year>=2005 and year<=2013){
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
					double survival = model.mortality_survival(size) * model.exploitation_survival(W,size);
					z += -log(survival);
				}
				z /= 3;
				// Store
				z_ests(year,quarter,z_size).expected = z;
			}
		}
	}

	/**
	 * Calculate the log-likelihood of the fits to the data
	 */
	double likelihood(void){
		double likelihood = 0;

		likelihood += maldive_pl_cpue.likelihood();
		likelihood += west_ps_cpue.likelihood();
		likelihood += size_freqs.likelihood();
		likelihood += z_ests.likelihood();

		return likelihood;
	}

	/**
	 * Read in observed data
	 */
	void read(void){
		maldive_pl_cpue.read_observed("data/processed-data/m-pl-cpue.tsv");
		west_ps_cpue.read_observed("data/processed-data/w-ps-cpue.tsv");
		size_freqs.read_observed_uncertainty("data/processed-data/size-frequencies.tsv");
		//z_ests.read_observed("data/processed-data/z-estimates.tsv");
	}

	/**
	 * Write out fits
	 */
	void write(void){
		maldive_pl_cpue.write("output/m-pl-cpue.tsv");
		west_ps_cpue.write("output/w-ps-cpue.tsv");
		size_freqs.write("output/size-freqs.tsv");
		z_ests.write("output/z-ests.tsv");
	}

}; // class Data

}
