#define DEBUG 0

#include "imports.hpp"
#include "dimensions.hpp"
#include "model.hpp"
#include "parameters.hpp" 
#include "data.hpp"
#include "procedures.hpp"
#include "tracker.hpp"

using namespace IOSKJ;

/**
 * Run the model with a parameters set read from "parameters/input"
 */
void run(void){
	boost::filesystem::create_directories("model/output");

	Model model;
	Parameters parameters;
	parameters.read();
	Data data;
	data.read();
	// Do tracking
	Tracker tracker("model/output/track.tsv");
	// For each time step...
	for(uint time=0;time<=time_max;time++){
		std::cout<<time<<"\t"<<year(time)<<"\t"<<quarter(time)<<std::endl;
		//... set model parameters
		parameters.set(model,time);
		//... update the model
		model.update(time);
		//... get model variables corresponding to data
		data.get(model,time);
		//... get model variables of interest for tracking
		tracker.get(model,time);
	}
	// Write out
	model.write();
	parameters.write();
	data.write();
}

/**
 * Generate samples from the parameter prior distributions 
 */
void priors(int replicates=1000){
	boost::filesystem::create_directories("parameters/output");

	Parameters parameters;
	parameters.read();
	Frame samples;
	for(int replicate=0;replicate<replicates;replicate++){
		parameters.randomise();
		samples.append(parameters.values());
	}
	samples.write("parameters/output/priors.tsv");
}

/**
 * Check feasibility constraints
 *
 * Used in the `condition _feasible` method
 */
int check_feasible(const Model& model, const Data& data, uint time, uint year, uint quarter){

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

/**
 * Condition based on feasibility constraints
 */
void condition_feasible(int trials=100){
	boost::filesystem::create_directories("feasible/output");

	Parameters parameters;
	parameters.read();
	Data data;
	data.read();
	// Frames for accepted and rejected parameter samples
	Frame accepted;
	Frame rejected;
	// Do a number of trial parameter samples
	for(int trial=0;trial<trials;trial++){
		parameters.randomise();
		Model model;
		uint time = 0;
		uint time_end = IOSKJ::time(2013,3);
		for(;time<=time_end;time++){
			// Do the time step
			//... set parameters
			parameters.set(model,time);
			//... update the model
			model.update(time);
			//... get data
			data.get(model,time);
			// Check feasibility
			uint year = IOSKJ::year(time);
			uint quarter = IOSKJ::quarter(time);
			int criterion = check_feasible(model,data,time,year,quarter);
			if(criterion!=0){
				Frame values = parameters.values();
				values.add("time",Integer,int(time));
				values.add("year",Integer,int(year));
				values.add("quarter",Integer,int(quarter));
				values.add("criterion",Integer,int(criterion));
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

void evaluate(int replicates=1000){
	boost::filesystem::create_directories("evaluate/output");
	// Setup parameters and data
	Parameters parameters;
	parameters.read();
	Data data;
	data.read();
	// Setup procedures
	Procedures procedures;
	// Read in samples from conditioning
	Frame samples;
	samples.read("feasible/output/accepted.tsv");
	samples.type<double>();
	// Create a frame for storing selected samples
	Frame selected;
	// For each replicate...
	for(int replicate=0;replicate<replicates;replicate++){
		std::cout<<replicate<<std::endl;
		// Randomly select a parameter sample
		Frame sample = samples.row(
			Uniform(0,samples.rows()).random()
		);
		selected.append(sample);
		// Read parameters from sample 
		// (to save time don't attempt to read catches array)
		parameters.read(sample,{"catches"});
		// Create a model representing current state by iterating
		// from time 0 to now...
		Model current;
		for(uint time=0;time<=time_now;time++){
			//... set parameters
			parameters.set(current,time);
			//... update the model
			current.update(time);
		}
		// Generate a random seed to be used to ensure future
		// variability is same for all procedures
		uint seed = Uniform(0,1e10).random();
		// For each candidate procedure...
		for(uint procedure=0;procedure<procedures.size();procedure++){
			// Create a model with current state to use to 
			// simulate procedure
			Model future = current;
			// Reset random seed
			Generator.seed(seed);
			// Reset the procedure
			procedures.reset(procedure);
			// Iterate over years...
			for(uint time=time_now+1;time<=time_max;time++){
				//... update the model
				future.update(time);
				//... operate the procedure
				procedures.operate(procedure,time,future);
			}
		}
	}
	selected.write("evaluate/output/samples.tsv");
}

int main(int argc, char** argv){ 
	try{
        if(argc==1) throw std::runtime_error("No task given");
        std::string task = argv[1];
        uint num = (argc>2)?boost::lexical_cast<uint>(argv[2]):0;
        std::cout<<"-------------"<<task<<"-------------\n";
        if(task=="run") run();
        else if(task=="priors") priors();
        else if(task=="feasible") condition_feasible(num);
        else if(task=="evaluate") evaluate(num);
        else throw std::runtime_error("Unrecognised task");
        std::cout<<"-------------------------------\n";
	} catch(std::exception& error){
        std::cout<<"************Error************\n"
                <<error.what()<<"\n"
                <<"******************************\n";
	} catch(...){
        std::cout<<"************Unknown error************\n";
	}
	return 0;
}
