#define DEBUG 1

#include "imports.hpp"
#include "dimensions.hpp"
#include "model.hpp"
#include "parameters.hpp"
#include "data.hpp"
#include "tracker.hpp"
 
using namespace IOSKJ;

// Instantiate helper classes
Parameters parameters;
Data data;

/**
 * Tasks that need to be done at startup
 */
void startup(void){
	// Read in parameters (including catches)
	parameters.read();
	// Read in data
	data.read();
}

/**
 * Run the model with a parameters set read from the "parameters.tsv" file
 */
void run_pars(void){
	Model model;
	// Read in parameter values (and write out for checking)
	auto parameter_set = parameters.read_set("parameters.tsv");
	parameters.write_set("output/parameters_read.tsv",parameter_set);
	// Do tracking
	Tracker tracker("output/track.tsv");
	// For each time step...
	for(uint time=0;time<=time_max;time++){
		#if DEBUG
		std::cout<<time<<" "<<year(time)<<" "<<quarter(time)<<" "<<model.biomass_spawning_overall(quarter(time))<<std::endl;
		#endif
		//... set model parameters
		parameters.set(model,time,parameter_set);
		//... update the model
		model.update(time);
		//... get model variables corresponding to data
		data.get(model,time);
		//... get model variables of interest for tracking
		tracker.get(model,time);
	}
}

/**
 * Do slices of likelihoods for a parameter
 */
void profile(std::string parameter){ 
	auto set = parameters.read_set("parameters.tsv");
	parameters.profile(parameter,set,data,"profile.tsv");
}

/**
 * Tasks that are usually done at shutdown
 */
void shutdown(void) {
	// Output parameter details
	parameters.write();
	// Output data
	data.write();
}


int main(void){
	startup();
	try{
		run_pars();
		//profile("recruits_steepness");
		profile("recruits_unfished");
		//profile("growth_assymptote");
	} catch(std::exception& e){
		std::cout<<e.what()<<std::endl;
	}
	shutdown();
	return 0;
}
