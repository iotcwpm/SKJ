#pragma once

#include "model.hpp"
#include "data.hpp"

namespace IOSKJ {

class Procedure {
public:
	virtual void reset(void){
	};
	virtual void operate(uint time, Model& model){
	};
};

class ProcedureA : public Procedure, public Structure<ProcedureA> {
public:

	/**
	 * Precision with which B (stock status) is estimated
	 */
	double B_precision;

	/**
	 * Target (maximum) F (fishing mortality)
	 */
	double F_target;

	/**
	 * Theshold B (stock status) below which F is reduced
	 */
	double B_thresh;

	/**
	 * Limit B (stock status) below which F is 0
	 */
	double B_limit;

	/**
	 * Reflection
	 */
	template<class Mirror>
	void reflect(Mirror& mirror){
		mirror
			.data(B_precision,"B_precision")
			.data(F_target,"F_target")
			.data(B_thresh,"B_thresh")
			.data(B_limit,"B_limit")
		;
	}

	double f_calc(double b){
	    if(b<B_limit) return 0;
    	else if(b>B_thresh) return F_target;
    	else return F_target/(B_thresh-B_limit)*(b-B_limit);
    }

	virtual void operate(uint time, Model& model){
		// Get stock status
		double b = model.biomass_status(time);
		// Add imprecision
		Lognormal imprecision(1,B_precision);
		b *= imprecision.random();
		// Calculate F
		double f = f_calc(b);
		// Apply F
		model.fishing_mortality_set(f);
	}

};

class Procedures : public Array<Procedure*> {
public:

	void reset(int procedure){
		operator[](procedure)->reset();
	}

	void operate(int procedure, uint time, Model& model){
		operator[](procedure)->operate(time,model);
	}
};

}
