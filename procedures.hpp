#pragma once

#include "model.hpp"
#include "data.hpp"

namespace IOSKJ {

class Procedure {
public:
	virtual void reset(void) = 0;
	virtual void operate(uint time, Model& model) = 0;
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

    void reset(void){
    }

    void operate(uint time, Model& model){
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
