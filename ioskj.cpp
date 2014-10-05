#define DEBUG 1

#include "imports.hpp"
#include "dimensions.hpp"
#include "model.hpp"
#include "parameters.hpp" 
#include "data.hpp"
#include "tracker.hpp"

using namespace IOSKJ;

// Instantiate components
Model model;
Parameters parameters;
Data data;

/**
 * Tasks that need to be done at startup
 */
void startup(void){
	// Read parameters and data
	parameters.read();
	data.read();
}

/**
 * Tasks that are usually done at shutdown
 */
void shutdown(void) {
	// Write parameters and data
	parameters.write();
	data.write();
	// Write model
	model.write();
}

/**
 * Run the model with a parameters set read from "parameters/input"
 */
void run(void){

	// Do tracking
	Tracker tracker("model/output/track.tsv");
	// For each time step...
	for(uint time=0;time<=time_max;time++){
		#if DEBUG
		std::cout<<time<<" "<<year(time)<<" "<<quarter(time)<<" "<<model.biomass_spawning_overall(quarter(time))<<std::endl;
		#endif
		//... set model parameters
		parameters.set(model,time);
		//... update the model
		model.update(time);
		//... get model variables corresponding to data
		data.get(model,time);
		//... get model variables of interest for tracking
		tracker.get(model,time);
	}
}

void priors(int replicates=1000){
	Frame<> samples = parameters.sample(replicates);
	std::cout<<samples.rows();
	samples.write("parameters/output/priors.tsv");
}

int main(int argc, char** argv){ 
	startup();
	try{
		for(int arg=1;arg<argc;arg++){
            std::string task = argv[arg];
            std::cout<<"-------------Task-------------\n"
                    <<task<<"\n"
                    <<"-------------------------------\n";
            if(task=="run") run();
            else if(task=="priors") priors();
        }
	} catch(std::exception& error){
        std::cout<<"************Error************\n"
                <<error.what()<<"\n"
                <<"******************************\n";
	} catch(...){
        std::cout<<"************Unknown error************\n";
	}
	shutdown();
	return 0;
}
