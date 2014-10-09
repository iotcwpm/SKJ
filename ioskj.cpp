#define DEBUG 0

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
void step(uint time){
	//... set model parameters
	parameters.set(model,time);
	//... update the model
	model.update(time);
	//... get model variables corresponding to data
	data.get(model,time);
}

/**
 * Run the model with a parameters set read from "parameters/input"
 */
void run(void){
	// Do tracking
	Tracker tracker("model/output/track.tsv");
	// For each time step...
	for(uint time=0;time<=time_max;time++){
		//... run steps
		step(time);
		//... get model variables of interest for tracking
		tracker.get(model,time);
	}
}

void priors(int replicates=1000){
	Frame<> samples = parameters.sample(replicates);
	samples.write("parameters/output/priors.tsv");
}

int check_feasible(uint time, uint year, uint quarter){

	// Stock status ...
	auto status = model.biomass_spawning_overall(quarter)/model.biomass_spawning_unfished(quarter);
	// ... must always be >10% B0
	if(status<0.1) return 1;
	// ... since 2008 must be less than 100% B0
	if(year>2008 and status>1) return 2;

	// M PL CPUE ...
	static double m_pl_cpue_base;
	if(year==2004 and quarter==2) m_pl_cpue_base = data.m_pl_cpue(year,quarter);
	// ... must have increased from 2004 to 2006
	if(year==2006 and quarter==2 and data.m_pl_cpue(year,quarter)/m_pl_cpue_base<1) return 3;
	// ... must have decreased from 2004 to 2011
	if(year==2011 and quarter==2 and data.m_pl_cpue(year,quarter)/m_pl_cpue_base>1) return 4;

	return 0;
}

void condition_feasible(int trials=100){
	auto labels = parameters.labels();
	Frame<> accepted(labels);

	Frame<> rejected(labels);
	rejected.add("time",Integer);
	rejected.add("year",Integer);
	rejected.add("quarter",Integer);
	rejected.add("criterion",Integer);

	for(int trial=0;trial<trials;trial++){
		parameters.randomise();
		uint time = 0;
		uint time_end = IOSKJ::time(2013,3);
		for(;time<=time_end;time++){
			// Do the time step
			step(time);
			// Check feasibility
			uint year = IOSKJ::year(time);
			uint quarter = IOSKJ::quarter(time);
			int criterion = check_feasible(time,year,quarter);
			if(criterion!=0){
				auto values = parameters.values();
				values.push_back(time);
				values.push_back(year);
				values.push_back(quarter);
				values.push_back(criterion);
				rejected.append(values);
				break;
			}
			if(time==time_end){
				accepted.append(parameters.values());
			}
		}
		if(trial>0 and trial%10==0) std::cout<<trial<<" "<<accepted.rows()/float(trial)<<std::endl;
	}
	accepted.write("feasible/output/accepted.tsv");
	rejected.write("feasible/output/rejected.tsv");
}

int main(int argc, char** argv){ 
	startup();
	try{
        if(argc==1) throw std::runtime_error("No task given");
        std::string task = argv[1];
        uint num = (argc>2)?boost::lexical_cast<uint>(argv[2]):0;
        std::cout<<"-------------Task-------------\n"<<task<<"\n";
        if(task=="run") run();
        else if(task=="priors") priors();
        else if(task=="feasible") condition_feasible(num);
        std::cout<<"-------------------------------\n";
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
