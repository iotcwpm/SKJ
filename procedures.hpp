#pragma once

#include "model.hpp"
#include "data.hpp"

namespace IOSKJ {

class Procedure {
public:
	virtual void reset(void){};
	virtual void operate(uint time, Model& model) = 0;
	virtual void write(std::ofstream& stream) = 0;

	/**
	 * Set the catch by region/method assuming a 
	 * certain allocation, currently based on the 
	 * period 2003-2012 (see `data/nominal-catches-quarter.R`)
	 */
	void catches_set(Model& model,double catches){
		model.exploit = model.exploit_catch;

		model.catches(W,PS) = 0.354 * catches;
		model.catches(W,PL) = 0.018 * catches;
		model.catches(W,GN) = 0.117 * catches;
		model.catches(W,OT) = 0.024 * catches;

		model.catches(M,PS) = 0.000 * catches;
		model.catches(M,PL) = 0.198 * catches;
		model.catches(M,GN) = 0.000 * catches;
		model.catches(M,OT) = 0.005 * catches;

		model.catches(E,PS) = 0.058 * catches;
		model.catches(E,PL) = 0.006 * catches;
		model.catches(E,GN) = 0.141 * catches;
		model.catches(E,OT) = 0.078 * catches;
	}

	/**
	 * Set the number of effort units by region/method
	 */
	void effort_set(Model& model,double effort){
		model.exploit = model.exploit_effort;
		// Since effort units are currently nominal
		// for each region/method relative to the period
		// 2004-2013, effort is set the same for all region/methods
		model.effort = effort;
	}
};

/**
 * `DoNothing` management procedure
 *
 * A management procedure that does nothing; used for testing
 */
class DoNothing : public Procedure, public Structure<DoNothing> {
public:

	virtual void write(std::ofstream& stream){
		stream
			<<"DoNothing"<<"\t\t\t\t\t\t\t\t\t\t\n";
	}

	virtual void operate(uint time, Model& model){
	}
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

	virtual void operate(uint time, Model& model){
		// Get stock status
		double b = model.biomass_status(time);
		// Add imprecision
		Lognormal imprecision(1,precision);
		b *= imprecision.random();
		// Calculate F
		double f;
		if(b<limit) f = 0;
		else if(b>thresh) f = target;
		else f = target/(thresh-limit)*(b-limit);
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
		effort = 100;
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
				double adjust = target/f;
				// Adjust effort
				effort *= adjust;
				effort_set(model,effort);
			}
			last_ = year;
		}
	}

private:

	/**
	 * Time that the last F estimate was made
	 */
	int last_;

	/**
	 * Total allowable effort
	 */
	double effort;
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
			// Apply recommended TAC
			catches_set(model,tac);
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
		// DoNothing
		append(new DoNothing);
		append(new DoNothing);
		/*!!!!!!!!!!!!
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
		*/
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
