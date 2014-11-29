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
	 * Frequency of stock status estimates
	 */
	int frequency = 2;

	/**
	 * Precision with which B (stock status) is estimated
	 */
	double precision = 0.1;

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
			.data(frequency,"frequency")
			.data(precision,"precision")
			.data(target,"target")
			.data(thresh,"thresh")
			.data(limit,"limit")
		;
	}

	void write(std::ofstream& stream){
		stream
			<<"BRule"<<"\t"
			<<frequency<<"\t"
			<<precision<<"\t"
			<<target<<"\t"
			<<thresh<<"\t"
			<<limit<<"\t\t\t\t\t\n";
	}

	virtual void reset(void){
		last_ = -1;
	}

	virtual void operate(uint time, Model& model){
		int year = IOSKJ::year(time);
		int quarter = IOSKJ::quarter(time);
		if(quarter==3 and (last_<0 or year-last_>frequency)){
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
	}

private:
	/**
	 * Time that the status estimate was made
	 */
	int last_;
};

/**
 * `FRange` management procedure
 */
class FRange : public Procedure, public Structure<FRange> {
public:

	/**
	 * Frequency of exp. rate estimates
	 */
	int frequency = 2;

	/**
	 * Precision of exp. rate estimate
	 */
	double precision = 0.1;

	/**
	 * Target exp. rate
	 */
	double target;

	/**
	 * Buffer around target exp. rate
	 */
	double buffer;

	/**
	 * Restriction on multiplicative changes
	 * in effort
	 */
	double change_max = 0.3;

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
			.data(change_max,"change_max")
		;
	}

	void write(std::ofstream& stream){
		stream
			<<"FRange"<<"\t"
			<<frequency<<"\t"
			<<precision<<"\t"
			<<target<<"\t"
			<<buffer<<"\t"
			<<change_max<<"\t\t\t\t\t\n";
	}

	virtual void reset(void){
		last_ = -1;
		effort = 100;
	}

	virtual void operate(uint time, Model& model){
		int year = IOSKJ::year(time);
		int quarter = IOSKJ::quarter(time);
		if(quarter==3 and (last_<0 or year-last_>frequency)){
			// Get an estimate of exploitation rate
			double f = model.exploitation_rate_get();
			// Add imprecision
			Lognormal imprecision(1,precision);
			f *= imprecision.random();
			// Check to see if F is outside of range
			if(f<target-buffer or f>target+buffer){
				// Calculate ratio between current estimated F and target
				double adjust = target/f;
				if(adjust>(1+change_max)) adjust = 1+change_max;
				else if(adjust<1/(1+change_max)) adjust = 1/(1+change_max);
				// Adjust effort
				effort *= adjust;
				effort_set(model,effort);
			}
			last_ = year;
		}
	}

private:

	/**
	 * Time that the last exp. rate estimate was made
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
	double responsiveness = 1;

	/**
	 * Target harvest rate relative to historic levels i.e 0.9 = 90% of historic average
	 */
	double multiplier = 400000;

	/**
	 * Threshold biomass index
	 */
	double threshold = 0.3;

	/**
	 * Limit biomass index
	 */
	double limit = 0.1;

	/**
	 * Maximum change
	 */
	double change_max = 0.3;

	/**
	 * Buffer around target F
	 */
	double maximum = 600000;

	/**
	 * Reflection
	 */
	template<class Mirror>
	void reflect(Mirror& mirror){
		mirror
			.data(precision,"precision")
			.data(responsiveness,"responsiveness")
			.data(multiplier,"multiplier")
			.data(threshold,"threshold")
			.data(limit,"limit")
			.data(change_max,"change_max")
			.data(maximum,"maximum")
		;
	}

	void write(std::ofstream& stream){
		stream
			<<"IRate"<<"\t"
			<<precision<<"\t"
			<<responsiveness<<"\t"
			<<multiplier<<"\t"
			<<threshold<<"\t"
			<<limit<<"\t"
			<<change_max<<"\t"
			<<maximum<<"\t\t\t\n";
	}

	virtual void reset(void){
		last_ = -1;
	}

	virtual void operate(uint time, Model& model){
		int quarter = IOSKJ::quarter(time);
		// Operate once per year in the third quarter
		if(quarter==3){
			// Get CPUE as a combination of W/PS and M/PL
			GeometricMean combined;
			combined.append(model.cpue(W,PS));
			combined.append(model.cpue(M,PL));
			double cpue = combined;
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
			// Restrict changes in TAC
			if(last_!=-1){
				double change = tac/last_;
				double max = 1 + change_max;
				if(change>max) change = max;
				else if(change<1/max) change = 1/max;
				tac = last_*change;
			}
			last_ = tac;
			// Apply recommended TAC
			catches_set(model,tac);
		}
	}

private:

	/**
	 * Smoothed biomass index
	 */
	double index_;

	/**
	 * Last TAC value
	 */
	double last_;
};


class Procedures : public Array<Procedure*> {
public:

	void populate(void){
		// DoNothing procedure mainly for testing
		append(new DoNothing);
		// Examples of each MP mainly for testing
		{
			auto& proc = * new BRule;
			proc.precision = 0.1;
			proc.target = 0.2;
			proc.thresh = 0.6;
			proc.limit = 0.1;
			append(&proc);
		}
		{
			auto& proc = * new FRange;
			proc.frequency = 3;
			proc.precision = 0.1;
			proc.target = 0.25;
			proc.buffer = 0.05;
			proc.change_max = 0.3;
			append(&proc);
		}
		{
			auto& proc = * new IRate;
			proc.responsiveness = 0.6;
			proc.multiplier = 415000;
			proc.threshold = 0.4;
			proc.limit = 0.1;
			proc.change_max = 0.3;
			append(&proc);
		}

		// BRule
		for(int frequency : {1,2,5}){
			for(double precision : {0.0,0.1,0.2}){
				for(auto target : {0.2,0.25,0.3}){
					for(auto thresh : {0.6,0.7}){
						for(auto limit : {0.05,0.1,0.2}){
							auto& proc = * new BRule;
							proc.frequency = frequency;
							proc.precision = precision;
							proc.target = target;
							proc.thresh = thresh;
							proc.limit = limit;
							append(&proc);
						}
					}
				}
			}
		}
		// FRange
		for(int frequency : {1,2,5}){
			for(double precision : {0.0,0.1}){
				for(auto target : {0.2,0.25,0.3}){
					for(auto buffer : {0.01,0.02,0.05}){
						auto& proc = * new FRange;
						proc.frequency = frequency;
						proc.precision = precision;
						proc.target = target;
						proc.buffer = buffer;
						proc.change_max = 0.4;
						append(&proc);
					}
				}
			}
		}
		// IRate
		for(double responsiveness : {0.65, 1.0}){
			for(double multiplier : {400000,450000}){
				for(auto threshold : {0.6, 0.7}){
					for(auto limit : {0.1, 0.2}){
						auto& proc = * new IRate;
						proc.precision = 0.2;
						proc.responsiveness = responsiveness;
						proc.multiplier = multiplier;
						proc.threshold = threshold;
						proc.limit = limit;
						proc.change_max = 0.4;
						append(&proc);
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
