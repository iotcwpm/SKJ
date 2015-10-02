#define DEBUG 0

#include "imports.hpp"
#include "dimensions.hpp"
#include "model.hpp"
#include "parameters.hpp" 
#include "data.hpp"
#include "procedures.hpp"
#include "performance.hpp"
#include "tracker.hpp"

using namespace IOSKJ;

/**
 * Run the model with a parameters set read from "parameters/input"
 *
 * @param samples_file A filesystem path to a TSV file of parameter samples
 * @parameters samples_row Row index of samples to select
 */
void run(const std::string& samples_file="ref",int samples_row=0,int procedure=0){
	// Create output directories
	boost::filesystem::create_directories("model/output");
	boost::filesystem::create_directories("parameters/output");
	boost::filesystem::create_directories("data/output");
	// Read in parameters
	Parameters parameters;
	parameters.read();
	parameters.write();
	// If samples is specified, read them in and select the desired row
	if(samples_file!="ref"){
		// Read in samples
		Frame samples;
		samples.read(samples_file);
		//... select desired row
		Frame row = samples.slice(samples_row);
		//... overwrite parameters available, ignoring catches
		parameters.read(row,{"catches"});
	}
	// Read in data
	Data data;
	data.read();
	// Setup procedures
	Procedures procedures;
	procedures.populate();
	// Do tracking
	Tracker tracker("model/output/track.tsv");
	// Instantiate a model
	Model model;
	// For each time step...
	for(uint time=0;time<=time_max;time++){
		std::cout<<time<<"\t"<<year(time)<<"\t"<<quarter(time)<<std::endl;
		//... set model parameters
		parameters.set(time,model);
		//... update the model
		model.update(time);
		//... operate the procedure
		if(time>time_now){
			if(time==time_now+1) procedures.reset(procedure);
			procedures.operate(procedure,time,model);
		}
		//... get model variables corresponding to data
		data.get(time,model);
		//... get model variables of interest for tracking
		tracker.get(0,0,time,model);
	}
	// Write out
	model.write();
	parameters.write();
	data.write();
}

/**
 * Perform yield curve and MSY calculations with a parameters set read from "parameters/input"
 */
void yield(void){
	boost::filesystem::create_directories("yield/output");

	Model model;
	// Read in and set parameters
	Parameters parameters;
	parameters.read();
	parameters.set(0,model);
	// Generate and output yield curve
	model.yield_curve().write("yield/output/curve.tsv");
	// Go to Bmsy (to get catches by region/method) and output
	model.msy_go();
	Frame msy(
		{
			"e_msy","f_msy","msy","biomass_spawners_msy","biomass_spawners_unfished","msy_trials",
			"msy_total","msy_w_ps","msy_m_pl","msy_e_gn"
		},
		{
			model.e_msy,model.f_msy,model.msy,model.biomass_spawners_msy,model.biomass_spawners_unfished,double(model.msy_trials),
			model.catches_taken(sum),model.catches_taken(SW,PS)+model.catches_taken(NW,PS),model.catches_taken(M,PL),model.catches_taken(E,GN)
		}
	);
	msy.write("yield/output/msy.tsv");
	// Generate and output yield per recruit curve
	model.yield_per_recruit().write("yield/output/per_recruit.tsv");
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
 * Check feasibility constraints for a model
 */
typedef int (Check)(const Model& model, const Data& data, uint time, uint year, uint quarter);
void check(Check check, int trial, Parameters& parameters, Data& data, Tracker& tracker, Frame& accepted, Frame& rejected){
	Model model;
	uint time = 0;
	uint time_end = IOSKJ::time(2013,3);
	for(;time<=time_end;time++){
		// Do the time step
		//... set parameters
		parameters.set(time,model);
		//... update the model
		model.update(time);
		//... get data
		data.get(time,model);
		//... do tracking
		if(trial<100) tracker.get(trial,-1,time,model);
		//... check model
		uint year = IOSKJ::year(time);
		uint quarter = IOSKJ::quarter(time);
		int criterion = check(model,data,time,year,quarter);
		if(criterion!=0){
			Frame values = parameters.values();
			values.add("time",time);
			values.add("year",year);
			values.add("quarter",quarter);
			values.add("criterion",criterion);
			rejected.append(values);
			break;
		}
		if(time==time_end){
			accepted.append(parameters.values());
		}
	}
	if(trial>0 and trial%10==0) std::cout<<trial<<" "<<accepted.rows()/float(trial)<<std::endl;
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

	// Z-estimates
	if(year>=2006 and year<=2009){
		if(data.z_ests(year,quarter,1)<0.2 or data.z_ests(year,quarter,1)>0.7) return 5;
	}

	return 0;
}

/**
 * Condition based on feasibility constraints
 */
void condition_feasible(int trials=100){
	// Create output directory
	boost::filesystem::create_directories("feasible/output");
	// Read in parameter priors and default values
	Parameters parameters;
	parameters.read();
	// Read in data
	Data data;
	data.read();
	// Frames for accepted and rejected parameter samples
	Frame accepted;
	Frame rejected;
	// Do tracking (for a subset of trials)
	Tracker tracker("feasible/output/track.tsv");
	// Do a number of trial parameter samples
	for(int trial=0;trial<trials;trial++){
		// Randomly sample parameters from priors
		parameters.randomise();
		// Check feasibility of parameters
		check(check_feasible,trial,parameters,data,tracker,accepted,rejected);
	}
	// Write out
	accepted.write("feasible/output/accepted.tsv");
	rejected.write("feasible/output/rejected.tsv");
}

/**
 * Check SS3 model run
 *
 * Used in the `condition_ss3` method
 */
int check_ss3(const Model& model, const Data& data, uint time, uint year, uint quarter){
	// Stock status ...
	auto status = model.biomass_status(time);
	// ... must always be >10% B0
	if(status<0.1) return 1;
	// ... since 2008 must be less than 100% B0
	if(year>=2008 and status>1) return 2;

	// Exploitation rate must be within broad range
	// for each of the main region/method combinations. 
	// This constraint is to prevent infeasible combinations
	// of `recruits_region` and movement parameters
	if(year>=2005){
		if(
			model.exploitation_rate(SW,PS)>0.5 or
			model.exploitation_rate(NW,PS)>0.5 or
			model.exploitation_rate(M,PL)>0.5 or
			model.exploitation_rate(E,GN)>0.5 
		) return 3;
	}

	return 0;
}

/**
 * Generate samples based on the SS3 assessment grid with priors used for those
 * parameters not available from there.
 */
void condition_ss3(int replicates=1000){
	// Create output directory
	boost::filesystem::create_directories("ss3/output");
	// Read in parameter priors and default values
	Parameters parameters;
	parameters.read();
	// Read in data
	Data data;
	data.read();
	// Read in parameter values from SS3 grid
	Frame grid;
	grid.read("ss3/pars.tsv");
	// Frames for accepted and rejected parameter samples
	Frame accepted;
	Frame rejected;
	// Do tracking (for a subset of trials)
	Tracker tracker("ss3/output/track.tsv");
	// For each replicate...
	for(int replicate=0;replicate<replicates;replicate++){
		//... randomise parameter values from priors
		parameters.randomise();
		//... randomly choose a grid cell
		Frame cell = grid.slice(
			Uniform(0,grid.rows()).random()
		);
		//... overwrite parameters avialable from grid
		parameters.read(cell,{"catches"});
		//... check feasibility of parameters
		check(check_ss3,replicate,parameters,data,tracker,accepted,rejected);
	}
	accepted.write("ss3/output/accepted.tsv");
	rejected.write("ss3/output/rejected.tsv");
}

/**
 * Evaluate management procedures
 *
 * @param vary Should replicates vary? Should only be set to false for testing
 * @param msy Should msy be calculated for each replicate?
 */
void evaluate(const std::string& samples_file, int replicates=1000, int procedure_select=-1, bool vary=true, bool msy=true){
	boost::filesystem::create_directories("evaluate/output");
	// Setup parameters and data
	Parameters parameters;
	parameters.read();
	Data data;
	data.read();
	// Read in samples from conditioning and 
	// create a frame for storing selected samples
	Frame samples_all;
	samples_all.read(samples_file);
	samples_all.write("evaluate/output/samples_all.tsv");
	Frame samples;
	// Frame for hoding reference points
	Frame references({
		"b0","e_msy","f_msy","msy","b_msy"
	});
	// Setup procedures
	Procedures procedures;
	procedures.populate();
	// Setup performance statistics
	Array<Performance> performances;
	// Do tracking (for a subset of replicates)
	Tracker tracker("evaluate/output/track.tsv");
	// For each replicate...
	for(int replicate=0;replicate<replicates;replicate++){
		std::cout<<replicate<<std::endl;
		// Randomly select a parameter sample
		Frame sample = samples_all.slice(
			vary?Uniform(0,samples_all.rows()).random():0
		);
		samples.append(sample);
		// Read parameters from sample 
		// (to save time don't attempt to read catches array)
		parameters.read(sample,{"catches"});
		// Generate a random seed to be used to ensure any stochastic
		// variations is same for all procedures. Placed here so, if necessary
		// can be made constant for all replicates for testing purposes
		uint seed = vary?Uniform(0,1e10).random():10000;
		Generator.seed(seed);
		// Create a model representing current state by iterating
		// from time 0 to now...
		Model current;
		for(uint time=0;time<=time_now;time++){
			//... set parameters
			parameters.set(time,current); 
			//... update the model
			current.update(time);
			//... track the model (for speed, only track some replicates)
			if(replicate<100) tracker.get(replicate,-1,time,current);
		}
		// Determine MSY related reference points
		if(msy) current.msy_find();
		// Record reference points for replicate
		references.append({
			current.biomass_spawners_unfished,
			current.e_msy,
			current.f_msy,
			current.msy,
			current.biomass_spawners_msy,
		});
		// For each candidate procedure...
		uint procedure_begin = 0;
		uint procedure_end = procedures.size()-1;
		if(procedure_select>=0){
			procedure_begin = procedure_select;
			procedure_end = procedure_select;
		}
		for(uint procedure=procedure_begin;procedure<=procedure_end;procedure++){
			// Create a model with current state to use to 
			// simulate procedure
			Model future = current;
			// Set up performance statistics
			Performance performance(replicate,procedure);
			// Reset random seed
			Generator.seed(seed);
			// Reset the procedure
			procedures.reset(procedure);
			// Iterate over years...
			for(uint time=time_now+1;time<=time_max;time++){
				//... set parameters
				parameters.set(time,current);
				//... update the model
				future.update(time);
				//... track the model (for speed, only some replicates)
				if(replicate<100) tracker.get(replicate,procedure,time,future);
				//... operate the procedure
				procedures.operate(procedure,time,future);
				//... record performance
				performance.record(time,future);
			}
			// Save performance
			performances.append(performance);
		}

		// Write out every 10 replicates (so results stored if aborted)
		// or if at end
		if(replicate%10==0 or replicate==replicates-1){
			procedures.write("evaluate/output/procedures.tsv");
			samples.write("evaluate/output/samples.tsv");
			references.write("evaluate/output/references.tsv");
			performances.write("evaluate/output/performances.tsv");
		}
	}
}

template<typename Type>
Type arg(int argc, char** argv, int which){
	if(argc<=which) return Type();
	return boost::lexical_cast<Type>(argv[which]);
}

int main(int argc, char** argv){ 
	try {
        if(argc==1) throw std::runtime_error("No task given");
        std::string task = argv[1];
        std::cout<<"-------------"<<task<<"-------------\n";
        if(task=="run") run(arg<std::string>(argc,argv,2),arg<int>(argc,argv,3),arg<int>(argc,argv,4));
        else if(task=="yield") yield();
        else if(task=="priors") priors();
        else if(task=="condition_feasible") condition_feasible(arg<int>(argc,argv,2));
        else if(task=="condition_ss3") condition_ss3(arg<int>(argc,argv,2));
        else if(task=="evaluate_feasible") evaluate("feasible/output/accepted.tsv",arg<int>(argc,argv,2));
        else if(task=="evaluate_ss3") evaluate("ss3/output/accepted.tsv",arg<int>(argc,argv,2));
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
