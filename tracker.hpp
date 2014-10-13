#pragma once

#include "model.hpp"

namespace IOSKJ {

/**
 * Tracking of various model variables during simulation.
 * Mainly used in testing.
 */
struct Tracker {

	std::ofstream file;

	Tracker(std::string path){
		file.open(path);
		file
			<<"replicate\t"
			<<"procedure\t"
			<<"year\t"
			<<"quarter\t"
			<<"recruits_determ\t"
			<<"recruits_deviation\t"
			<<"recruits\t"
			<<"biomass_spawning_overall\t"
			<<"biomass_spawning_w\t"
			<<"biomass_spawning_m\t"
			<<"biomass_spawning_e\t"
			<<"catches_total\t"
			<<"catches_w_ps\t"
			<<"catches_m_pl\t"
			<<"catches_e_gn\t"
			<<"exploitation_survival_m_20\t"
			<<"biomass_vulnerable_m_pl\t"
			<<"exp_rate_m_pl\t"
			<<std::endl;
	}

	void get(int replicate, int procedure, int time, const Model& model){
		uint year = IOSKJ::year(time);
		uint quarter = IOSKJ::quarter(time);
		file
			<<replicate<<"\t"
			<<procedure<<"\t"
			<<year<<"\t"
			<<quarter<<"\t"
			<<model.recruits_determ<<"\t"
			<<model.recruits_deviation<<"\t"
			<<model.recruits<<"\t"
			<<model.biomass_spawning_overall(quarter)<<"\t"
			<<model.biomass_spawning(W)<<"\t"
			<<model.biomass_spawning(M)<<"\t"
			<<model.biomass_spawning(E)<<"\t"
			<<model.catches_taken(sum)<<"\t"
			<<model.catches_taken(W,PS)<<"\t"
			<<model.catches_taken(M,PL)<<"\t"
			<<model.catches_taken(E,GN)<<"\t"
			<<model.exploitation_survival(M,20)<<"\t"
			<<model.biomass_vulnerable(M,PL)<<"\t"
			<<model.exploitation_rate(M,PL)<<"\t"
			<<std::endl;
	}
};

} // namespace IOSKJ
