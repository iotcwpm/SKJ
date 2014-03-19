#define DEBUG 1

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
	// Read in data
	data.read();
}

/**
 * Run the model with parameters set at the means of their priors
 */
void run_with_means(void){
	Model model;
	// Get values of parameters at mean
	auto parameter_values = parameters.prior_means();
	// For each time step...
	for(auto time : times){
		//... set model parameters
		parameters.set(model,time,parameter_values);
		//... update the model
		model.update(time);
		//... get model variables corresponding to data
		data.get(model,time);
		//... get model variables of interest for tracking
		//tracker.get(model,time);
	}
}

/**
 * Tasks that are usually done at shutdown
 * to output results regardless of the task called
 */
void shutdown(void) {
	// Output data for checking that is was read in correctly
	data.write();
	// Output prior PDFs for plotting
	parameters.prior_pdfs("prior-pdfs.tsv");
}

int main(void){
	startup();
	run_with_means();
	shutdown();
	return 0;
}