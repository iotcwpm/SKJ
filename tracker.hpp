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

			<<"biomass_status\t"

			<<"biomass_spawners_sw\t"
			<<"biomass_spawners_nw\t"
			<<"biomass_spawners_ma\t"
			<<"biomass_spawners_ea\t"
			<<"biomass_spawning_overall\t"

			<<"biomass_spawning_sw\t"
			<<"biomass_spawning_nw\t"
			<<"biomass_spawning_ma\t"
			<<"biomass_spawning_ea\t"

			<<"biomass_vulnerable_sw_ps\t"
			<<"biomass_vulnerable_nw_ps\t"
			<<"biomass_vulnerable_ma_pl\t"
			<<"biomass_vulnerable_ea_gn\t"

			<<"catches_total\t"
			<<"catches_sw_ps\t"
			<<"catches_nw_ps\t"
			<<"catches_ma_pl\t"
			<<"catches_ea_gn\t"

			<<"effort_total\t"
			<<"effort_sw_ps\t"
			<<"effort_nw_ps\t"
			<<"effort_ma_pl\t"
			<<"effort_ea_gn\t"

			<<"exp_rate_sw_ps\t"
			<<"exp_rate_nw_ps\t"
			<<"exp_rate_ma_pl\t"
			<<"exp_rate_ea_gn\t"
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

			<<model.biomass_status(time)<<"\t"

			<<model.biomass_spawners(SW)<<"\t"
			<<model.biomass_spawners(NW)<<"\t"
			<<model.biomass_spawners(MA)<<"\t"
			<<model.biomass_spawners(EA)<<"\t"

			<<model.biomass_spawning_overall(quarter)<<"\t"
			<<model.biomass_spawning(SW)<<"\t"
			<<model.biomass_spawning(NW)<<"\t"
			<<model.biomass_spawning(MA)<<"\t"
			<<model.biomass_spawning(EA)<<"\t"

			<<model.biomass_vulnerable(SW,PS)<<"\t"
			<<model.biomass_vulnerable(NW,PS)<<"\t"
			<<model.biomass_vulnerable(MA,PL)<<"\t"
			<<model.biomass_vulnerable(EA,GN)<<"\t"
			
			<<model.catches_taken(sum)<<"\t"
			<<model.catches_taken(NW,PS)<<"\t"
			<<model.catches_taken(SW,PS)<<"\t"
			<<model.catches_taken(MA,PL)<<"\t"
			<<model.catches_taken(EA,GN)<<"\t"

			<<model.effort(sum)<<"\t"
			<<model.effort(SW,PS)<<"\t"
			<<model.effort(NW,PS)<<"\t"
			<<model.effort(MA,PL)<<"\t"
			<<model.effort(EA,GN)<<"\t"

			<<model.exploitation_rate(SW,PS)<<"\t"
			<<model.exploitation_rate(NW,PS)<<"\t"
			<<model.exploitation_rate(MA,PL)<<"\t"
			<<model.exploitation_rate(EA,GN)<<"\t"

			<<std::endl;
	}
};

} // namespace IOSKJ
