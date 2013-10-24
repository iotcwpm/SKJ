#pragma once

#include "data.hpp"
#include "fish.hpp"
#include "fishing.hpp"

namespace IOSKJ {

class Model {
public:

	Fish fish;
	Fishing fishing;
	
	#if TRACKING

		std::string track_filename;
		std::ofstream track_file;

		void track_begin(void){
			track_file.open(track_filename);

			track_file
				<<"time\t"
				<<"year\t"
				<<"quarter\t"
				<<std::endl;
		}

		void track(void){
			track_file
				<<time<<"\t"
				<<year<<"\t"
				<<quarter<<"\t";

			track_file<<std::endl;
		}

		void track_end(void){
			track_file.close();
		}

	#else

		void track_begin(void){
		}

		void track(void){
		}

		void track_end(void){
		}

	#endif

	void startup(void){
		Data::startup();

		fish.defaults();
		fishing.defaults();
	}

	void simulate(void){
		track_begin();

		for(time=0;time<times;time++){
			// Set Year and Quarter
			year = 1950 + time/4;
			quarter = time%4;

			// If time zero then initialise things
			if(time==0){
				fish.init();
				fishing.init();
			}

			// Otherwise do the usual annual time step
			else {
				fish.step();
				fishing.step();
			}

			track();
		}

		track_end();
	}

	void shutdown(void){
		Data::shutdown();
	}

};

}
