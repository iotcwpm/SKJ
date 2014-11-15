#pragma once

#include "model.hpp"
#include "data.hpp"

namespace IOSKJ {

class Procedure {
public:
	virtual void reset(void){};
	virtual void operate(uint time, Model& model) = 0;
	virtual void write(std::ofstream& stream) = 0;
};

/**
 * `BRule` management procedure
 */
class BRule : public Procedure, public Structure<BRule> {
public:

	/**
	 * Precision with which B (stock status) is estimated
	 */
	double precision;

	/**
	 * Target (maximum) F (fishing mortality)
	 */
	double target;

	/**
	 * Threshold B (stock status) below which F is reduced
	 */
	double thresh;

	/**
	 * Limit B (stock status) below which F is 0
	 */
	double limit;

	/**
	 * Reflection
	 */
	template<class Mirror>
	void reflect(Mirror& mirror){
		mirror
			.data(precision,"precision")
			.data(target,"target")
			.data(thresh,"thresh")
			.data(limit,"limit")
		;
	}

	void write(std::ofstream& stream){
		stream
			<<"BRule"<<"\t"
			<<precision<<"\t"
			<<target<<"\t"
			<<thresh<<"\t"
			<<limit<<"\t\t\t\t\t\t\n";
	}

	double f_calc(double b){
	    if(b<limit) return 0;
    	else if(b>thresh) return target;
    	else return target/(thresh-limit)*(b-limit);
    }

	virtual void operate(uint time, Model& model){
		// Get stock status
		double b = model.biomass_status(time);
		// Add imprecision
		Lognormal imprecision(1,precision);
		b *= imprecision.random();
		// Calculate F
		double f = f_calc(b);
		// Apply F
		model.fishing_mortality_set(f);
	}

};

/**
 * `FRange` management procedure
 */
class FRange : public Procedure, public Structure<FRange> {
public:

	/**
	 * Frequency of F estimates
	 */
	int frequency;

	/**
	 * Precision of F estimate
	 */
	double precision;

	/**
	 * Target F
	 */
	double target;

	/**
	 * Buffer around target F
	 */
	double buffer;

	/**
	 * Reflection
	 */
	template<class Mirror>
	void reflect(Mirror& mirror){
		mirror
			.data(frequency,"frequency")
			.data(precision,"precision")
			.data(target,"target")
			.data(buffer,"buffer")
		;
	}

	void write(std::ofstream& stream){
		stream
			<<"FRange"<<"\t"
			<<frequency<<"\t"
			<<precision<<"\t"
			<<target<<"\t"
			<<buffer<<"\t\t\t\t\t\t\n";
	}

	virtual void reset(void){
		last_ = -1; 
	}

	virtual void operate(uint time, Model& model){
		int year = IOSKJ::year(time);
		if(last_<0 or year-last_>frequency){
			// Get an estimate of F
			double f = model.biomass_status(time);
			// Add imprecision
			Lognormal imprecision(1,precision);
			f *= imprecision.random();
			// Check to see if F is outside of range
			if(f<target-buffer or f>target+buffer){
				// Calculate ratio between current estimated F and target
				// and adjust effort accordingly
				// double ratio = f/target;
				//! @todo Currently just setting to target F
				//! but should be adjusting effort
				model.exploitation_rate_set(f);
			}
			last_ = year;
		}
	}

private:

	/**
	 * Time that the last F estimate was made
	 */
	int last_;
};

/**
 * `IRate` management procedure
 */
class IRate : public Procedure, public Structure<IRate> {
public:

	/**
	 * Precision of CPUE in reflecting vulnerable biomass
	 */
	double precision = 0.2;

	/**
	 * Degree of smoothing of biomass index
	 */
	double responsiveness;

	/**
	 * Target harvest rate relative to historic levels i.e 0.9 = 90% of historic average
	 */
	double multiplier;

	/**
	 * Threshold biomass index
	 */
	double threshold;

	/**
	 * Limit biomass index
	 */
	double limit;

	/**
	 * Buffer around target F
	 */
	double maximum;

	/**
	 * Reflection
	 */
	template<class Mirror>
	void reflect(Mirror& mirror){
		mirror
			.data(responsiveness,"responsiveness")
			.data(multiplier,"multiplier")
			.data(threshold,"threshold")
			.data(limit,"limit")
			.data(maximum,"maximum")
		;
	}

	void write(std::ofstream& stream){
		stream
			<<"IRate"<<"\t"
			<<responsiveness<<"\t"
			<<multiplier<<"\t"
			<<threshold<<"\t"
			<<limit<<"\t"
			<<maximum<<"\t\t\t\t\t\n";
	}

	virtual void reset(void){
	}

	virtual void operate(uint time, Model& model){
		int year = IOSKJ::year(time);
		int quarter = IOSKJ::quarter(time);
		// Operate once per year in the third quarter
		if(quarter==3){
			// Get CPUE; currently for M-PL but could use any region/method
			double cpue = model.biomass_vulnerable(M,PL);
			// Add observation error
			Lognormal imprecision(1,precision);
			cpue *= imprecision.random();
			// Update smoothed index
			index_ = responsiveness*cpue + (1-responsiveness)*index_;
			// Calculate recommended harvest rate
			double rate;
			if(index_<limit) rate = 0;
			else if(index_>threshold) rate = multiplier;
			else rate = multiplier/(threshold-limit)*(index_-limit);
			// Calculate recommended TAC
			double tac = std::min(rate*cpue,maximum);	
					
		}
	}

private:

	/**
	 * Smoothed biomass index
	 */
	double index_;
};


class Procedures : public Array<Procedure*> {
public:

	void populate(void){
		// BRule
		for(double precision : {0.0,0.1}){
			for(auto target : {0.2,0.3}){
				for(auto thresh : {0.6,0.7}){
					for(auto limit : {0.1,0.2}){
						auto& proc = * new BRule;
						proc.precision = precision;
						proc.target = target;
						proc.thresh = thresh;
						proc.limit = limit;
						append(&proc);
					}
				}
			}
		}
		// FRange
		for(int frequency : {2,5}){
			for(double precision : {0.0,0.1}){
				for(auto target : {0.2,0.3}){
					for(auto buffer : {0.1,0.2}){
						auto& proc = * new FRange;
						proc.frequency = frequency;
						proc.precision = precision;
						proc.target = target;
						proc.buffer = buffer;
						append(&proc);
					}
				}
			}
		}
		// IRate
		for(double responsiveness : {0.65, 1.0}){
			for(double multiplier : {0.9, 1.0}){
				for(auto threshold : {0.6, 0.7}){
					for(auto limit : {0.1, 0.2}){
						for(auto maximum : {600}){
							auto& proc = * new IRate;
							proc.responsiveness = responsiveness;
							proc.multiplier = multiplier;
							proc.threshold = threshold;
							proc.limit = limit;
							proc.maximum = maximum;
							append(&proc);
						}
					}
				}
			}
		}
	}

	void reset(int procedure){
		operator[](procedure)->reset();
	}

	void operate(int procedure, uint time, Model& model){
		operator[](procedure)->operate(time,model);
	}

	void write(const std::string& path){
		std::ofstream file(path);
		file<<"procedure\tclass\tp1\tp2\tp3\tp4\tp5\tp6\tp7\tp8\tp9\tp10\n";
		int index = 0;
		for(Procedure* procedure : *this) {
			file<<index++<<"\t";
			procedure->write(file);
		}
	}
};

}
