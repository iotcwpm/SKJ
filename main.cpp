#define DEBUG 1
#define TRACKING 1

#include "common.hpp"
#include "dimensions.hpp"
#include "model.hpp"
#include "parameters.hpp"
#include "data.hpp"
 
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
	model.track_open("output/track.tsv");
	// Get values of parameters at mean
	auto parameter_set = parameters.read_set("parameters.tsv");
	// For each time step...
	for(uint time=0;time<=time_max;time++){
		std::cout<<time<<" "<<year(time)<<" "<<quarter(time)<<std::endl;
		//... set model parameters
		parameters.set(model,time,parameter_set);
		//... update the model
		model.update(time);
		//... get model variables corresponding to data
		data.get(model,time);
		//... get model variables of interest for tracking
		//tracker.get(model,time);
	}
	model.track_close();
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
	run_pars();
	shutdown();
	return 0;
}
