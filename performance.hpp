#pragma once

#include "model.hpp"

namespace IOSKJ {

/**
 * Performance statistics used for evaluating management procedures
 */
class Performance : public Structure<Performance> {
public:

	Performance(int replicate,int procedure):
		replicate(replicate),procedure(procedure){}

	/**
	 * Replicate evaluation which this performance 
	 * relates to
	 */
	int replicate;

	/**
	 * Candidate procedure which this performance 
	 * relates to
	 */
	int procedure;

	/**
	 * Number of time steps where performance measures
	 * were recorded. Mainly used for testing.
	 */
	Count times;

	/**
	 * Mean of total catch
	 */
	Mean catches_total;

	/**
	 * Mean of PS catch
	 */
	Mean catches_ps;

	/**
	 * Mean of PL catch
	 */
	Mean catches_pl;

	/**
	 * Mean of GN catch
	 */
	Mean catches_gn;

	/**
	 * Variance of catches
	 */
	Variance catches_var;

	/**
	 * Mean absolute of catches
	 */
	Mapc catches_mapc;

	/**
	 * Proportion of times where
	 * catch is zero
	 */
	Mean catches_shut;

	/**
	 * Mean of stock status % B0
	 */
	Mean status_mean;

	/**
	 * Probability of stock being below 10% BO
	 */
	Mean status_b10;

	/**
	 * Probability of stock being below 20% BO 
	 */
	Mean status_b20;

	/**
	 * Reflection
	 */
	template<class Mirror>
	void reflect(Mirror& mirror){
		mirror
			.data(replicate,"replicate")
			.data(procedure,"procedure")
			.data(times,"times")
			.data(catches_total,"catches_total")
			.data(catches_ps,"catches_ps")
			.data(catches_pl,"catches_pl")
			.data(catches_gn,"catches_gn")
			.data(catches_var,"catches_var")
			.data(catches_mapc,"catches_mapc")
			.data(catches_shut,"catches_shut")
			.data(status_mean,"status_mean")
			.data(status_b10,"status_b10")
			.data(status_b20,"status_b20")
		;
	}

	/**
	 * Record performance measures
	 */
	virtual void record(uint time, const Model& model){
		uint year = IOSKJ::quarter(year);
		uint quarter = IOSKJ::quarter(time);

		times.append();

		// Catch magnitude
		auto catch_total = model.catches_taken(sum);
		catches_total.append(catch_total);
		auto catches_by_method = model.catches_taken(sum,by(methods));
		catches_ps.append(catches_by_method(PS));
		catches_pl.append(catches_by_method(PL));
		catches_gn.append(catches_by_method(GN));

		// Catch variability
		if(catch_total>0){
			catches_var.append(catch_total);
			catches_mapc.append(catch_total);
		}
		catches_shut.append(catch_total==0);

		// Stock status
		auto status = model.biomass_spawning_overall(quarter)/model.biomass_spawning_unfished(quarter);
		status_mean.append(status);
		status_b10.append(status<0.1);
		status_b20.append(status<0.2);
	}

};

}
