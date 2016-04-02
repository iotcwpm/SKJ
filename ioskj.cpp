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

void tracks(const std::string& samples_file){
	// Read in parameters
	Parameters parameters;
	parameters.read();
	// Read in data
	Data data;
	data.read();
	// Read in samples
	Frame samples;
	samples.read(samples_file);
	// Do tracking
	Tracker tracker("model/output/track.tsv");
	for(uint row=0;row<samples.rows();row++){
		//Overwrite parameters available, but don't worry about catches
		parameters.read(samples.slice(row),{"catches"});
		// Instantiate a model
		Model model;
		// For each time step...
		for(uint time=0;time<=time_now;time++){
			//... set model parameters
			parameters.set(time,model);
			//... update the model
			model.update(time);
			//... get model variables corresponding to data
			data.get(time,model);
			//... get model variables of interest for tracking
			tracker.get(row,0,time,model);
		}
	}
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
			"msy_total","msy_we_ps","msy_ma_pl","msy_ea_gn"
		},
		{
			model.e_msy,model.f_msy,model.msy,model.biomass_spawners_msy,sum(model.biomass_spawners_unfished),double(model.msy_trials),
			model.catches_taken(sum),model.catches_taken(WE,PS),model.catches_taken(MA,PL),model.catches_taken(EA,GN)
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
	uint time_end = time_now;
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
		if(criterion!=0 or time==time_end){
			//... get parameters and associated likelihoods
			Frame values = parameters.values();
			values.add("pars_like",parameters.loglike());
			values.add("data_like",data.loglike());
			if(criterion!=0){
				values.add("trial",trial);
				values.add("time",time);
				values.add("year",year);
				values.add("quarter",quarter);
				values.add("criterion",criterion);
				rejected.append(values);
				break;
			}
			if(time==time_end){
				accepted.append(values);
			}
		}
	}
	if(trial>0 and trial%10==0) std::cout<<trial<<" "<<accepted.rows()/float(trial)<<std::endl;
}


/**
 * Check feasibility constraints
 *
 * Used in the `condition _feasible` method
 */
struct QuantileBounds {
	double lower[3];
	double upper[3];
};
Array<QuantileBounds,Method> feasible_sf_quantiles;
int check_feasible(const Model& model, const Data& data, uint time, uint year, uint quarter){
	
	// Stock status ...
	auto status = model.biomass_status();
	// ... must always be >10% B0
	if(status<0.1) return 1;
	// ... since 2008 must be less than 100% B0
	if(year>2008 and status>1) return 2;

	// Exploitation rate must be less than 0.5 for the main region/method combinations.
	if(
		model.exploitation_rate(WE,PS)>0.5 or
		model.exploitation_rate(MA,PL)>0.5 or
		model.exploitation_rate(EA,GN)>0.5 
	) return 3;

	// MA PL CPUE ...
	static double m_pl_cpue_base;
	if(year==2004 and quarter==2) m_pl_cpue_base = data.m_pl_cpue(year,quarter);
	// ... must have decreased from 2004 to 2011
	if(year==2011 and quarter==2 and data.m_pl_cpue(year,quarter)/m_pl_cpue_base>1) return 4;

	// W PS CPUE ...
	static double w_ps_cpue_base;
	if(year==2000 and quarter==3) w_ps_cpue_base = data.w_ps_cpue(year);
	// ... must have decreased from 2004 to 2011
	if(year==2011 and quarter==3 and data.w_ps_cpue(year)/w_ps_cpue_base>1) return 5;

	// Z-estimates
	if(year>=2006 and year<=2009){
		auto value = data.z_ests(year,quarter,0);
		if(value<0.1 or value>0.4) return 6;
	}

	// Size-frequencies
	if(year==2014){
		for(auto method : methods){
			// Calculate cumulative proportions over
			// all years and regions
			Array<double,Size> cumulative = 0;
			int years = 0;
			for(int year=1990;year<=2014;year++){
				years++;
				for(auto region : regions){
					double running = 0;
					for(auto size : sizes){
						running += data.size_freqs(year,quarter,region,method,size);
						cumulative(size) += running;
					}
				}
			}
			// Normalise over regions
			for(auto& item : cumulative) item /= regions.size() * years;
			// Determine quantiles
			double q10 = -1;
			double q50 = -1;
			double q90 = -1;
			for(auto size : sizes){
				if(q10<0 and cumulative(size)>=0.1) q10 = model.length_size(size);
				if(q50<0 and cumulative(size)>=0.5) q50 = model.length_size(size);
				if(q90<0 and cumulative(size)>=0.9) q90 = model.length_size(size);
			}
			// Check against constraints
			auto bounds = feasible_sf_quantiles(method);
			if(q10<bounds.lower[0] or q10>bounds.upper[0] or
			   q50<bounds.lower[1] or q50>bounds.upper[1] or
			   q90<bounds.lower[2] or q90>bounds.upper[2]) return 7;
		}
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
	parameters.write();
	// Read in data
	Data data;
	data.read();
	data.write();
	// Read in constraints specified in external files
	feasible_sf_quantiles.read("feasible/input/size_freqs_quantiles.tsv",[](std::istream& file, QuantileBounds& bounds){
		file
			>>bounds.lower[0]>>bounds.upper[0]
			>>bounds.lower[1]>>bounds.upper[1]
			>>bounds.lower[2]>>bounds.upper[2]
		;
	});
	feasible_sf_quantiles.write("feasible/output/size_freqs_quantiles.tsv",{},[](std::ostream& file, const QuantileBounds& bounds){
		file
			<<bounds.lower[0]<<"\t"<<bounds.upper[0]<<"\t"
			<<bounds.lower[1]<<"\t"<<bounds.upper[1]<<"\t"
			<<bounds.lower[2]<<"\t"<<bounds.upper[2]
		;
	});
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
	auto status = model.biomass_status();
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
			model.exploitation_rate(WE,PS)>0.5 or
			model.exploitation_rate(MA,PL)>0.5 or
			model.exploitation_rate(EA,GN)>0.5 
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

void condition_demc(uint generations,uint logging=1, uint saving=10){
    // Create output directory
	boost::filesystem::create_directories("demc/output");
	// Set up log file
	std::ofstream log_file("demc/output/log.tsv");
    std::ofstream errors_file("demc/output/errors.tsv");
    std::ofstream trace("demc/output/trace.tsv");

	// Read in parameter priors and default values
	Parameters parameters;
	parameters.read();
	// Read in data
	Data data;
	data.read();

	Uniform chance(0,1);
	Normal error(0,0.01);

	std::vector<std::vector<double>> population;
	std::vector<double> loglikes;
	auto names = parameters.names();
	auto columns = names.size();

    // Blending of donor parameter values (Ter Braak's Gamma)
	// Default is 2.38/sqrt(2*d) jumping to 1
	// every 10th generation
	double blending = 2.38/std::sqrt(2*columns);

	// Cross over probability
	// Proportion of parameters that taken
	// from mutation
	double crossing = 0.25;

    // Population size (Ter Braak's N)
	// Default is 2*d
	unsigned int size = 2*columns;

	double acceptance = 1;

	auto run = [&](){
    	// Calculate likelihood for candidate
    	double loglike = NAN;
        try {            

			Model model;
			for(uint time=0;time<=time_now;time++){
				// Do the time step
				//... set parameters
				parameters.set(time,model);
				//... update the model
				model.update(time);
				//... get data
				data.get(time,model);
			}
			// Calculate likelihood
			loglike = parameters.loglike() + data.loglike();

        } catch(const std::exception& e){
            errors_file<<e.what()<<"\n";
            parameters.values().write(errors_file);
            errors_file<<std::endl;
        } catch(...){
            errors_file<<"\"Unknown error\"\n";
            parameters.values().write(errors_file);
            errors_file<<std::endl;
        }
        return loglike;
    };

    while(population.size()<size){
    	parameters.randomise();
    	auto initial = parameters.vector();
    	auto loglike = run();
    	if(not std::isfinite(loglike)) continue;
		population.push_back(initial);
		loglikes.push_back(loglike);
    }

    uint generation = 1;
    while(generation<=generations){  

    	// Alter blending
    	if(acceptance>0.3) blending /= 0.9;
    	else if(acceptance<0.2) blending *= 0.9;
    	double blending_now;
	  	if(generation%10==0){
			blending_now = std::min(blending*5,1.0);
		} else {
			blending_now = blending;
		}

    	uint accepted = 0;
    	uint trials = 0;
    	for(uint chain=0; chain<size; chain++){
	    	std::vector<double> parent = population[chain];
	    	double parent_loglike = loglikes[chain];
	    	std::vector<double> child(columns);

            // Mutation
            unsigned int random_1_row = chance.random()*size;
            unsigned int random_2_row = chance.random()*size;
           	std::vector<double> random_1 = population[random_1_row];
            std::vector<double> random_2 = population[random_2_row];
            for(uint column=0;column<columns;column++){
                auto value = parent[column];
                child[column] = value + blending_now*(random_1[column]-random_2[column]) + error.random()*std::fabs(value);
            }

            // Cross-over
	        for(uint column=0;column<columns;column++){
                if(chance.random()<(1-crossing)){
                    child[column] = parent[column];
                }
            }

            // Set parameters
           	parameters.vector(child);
           	// Bounce parameters off their bounds
            parameters.bounce();
            // Get parameters back after bounce
			child = parameters.vector();

	        auto loglike = run();
	        if(not std::isfinite(loglike)) continue;

            double ratio = std::exp(loglike-parent_loglike);
            if(chance.random()<ratio){
                accepted++;
                for(uint column=0;column<columns;column++){
                	population[chain][column] = child[column];
                }
                loglikes[chain] = loglike;
                // Record trace
                if(trace.tellp()==0){
                	trace<<"chain\t";
                	for(auto name : names) trace<<name<<"\t";
                	trace<<"loglike"<<"\n";
                }
                trace<<chain<<"\t";
                for(uint column=0;column<columns;column++){
					trace<<child[column]<<"\t";
                }
                trace<<loglike<<"\n";
            }

	        trials++;
	    }
	    acceptance = accepted/double(trials);

    	// Record log
		if(generation%logging==0){
            if(log_file.tellp()==0) log_file<<"generation\trows\tworst\tmean\tbest\tacceptance\tblending"<<std::endl;
	    	// Update stats
	        uint rows = population.size();
	        double sum = 0;
	        double best = -INFINITY;
	        double worst = INFINITY;
	        for(uint row=0;row<rows;row++){
	            auto loglike = loglikes[row];
	            sum += loglike;
	            best = std::max(loglike,best);
	            worst = std::min(loglike,worst);
	        };
	        double mean = sum/rows;
            // Output to file
            log_file<<generation<<"\t"
            	<<rows<<"\t"
            	<<worst<<"\t"<<mean<<"\t"<<best<<"\t"
            	<<acceptance<<"\t"
            	<<blending<<std::endl;
        }

        // Save population
		if(generation==generations or generation%saving==0){
			std::ofstream save("demc/output/population.tsv");
			for(auto name : names) save<<name<<"\t";
			save<<"loglike"<<std::endl;
			uint index = 0;
			for(auto row : population){
				for(auto value : row){
					save<<value<<"\t";
				}
				save<<loglikes[index++]<<std::endl;
			}
		}

        generation++;
    }
}

/**
 * Evaluate management procedures
 *
 * @param vary Should replicates vary? Should only be set to false for testing
 * @param msy Should msy be calculated for each replicate?
 */
void evaluate(
	int replicates=1000, 
	const std::string& samples_file="feasible/output/accepted.tsv", 
	bool procedures_read=true, 
	int procedure_select=-1,
	int year_start=-1, 
	bool vary=true, 
	bool msy=true
){
	boost::filesystem::create_directories("evaluate/output");
	boost::filesystem::create_directories("procedures/output");
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
	// Frame for holding reference points
	Frame references({
		"b0","e_msy","f_msy","msy","b_msy"
	});
	// Setup procedures
	Procedures procedures;
	if(procedures_read) procedures.read();
	else procedures.populate();
	procedures.write();
	// Setup performance statistics
	Array<Performance> performances;
	// Do tracking (for a subset of replicates)
	Tracker tracker("evaluate/output/track.tsv");
	uint time_start;
	if(year_start<0) time_start = time_now;
	else time_start = time(year_start,3);
	// For each replicate...
	for(int replicate=0;replicate<replicates;replicate++){
		std::cout<<replicate<<std::endl;
		// Randomly select a parameter sample
		Frame sample = samples_all.slice(
			vary?Uniform(0,samples_all.rows()).random():0
		);
		// Read parameters from sample 
		// (to save time don't attempt to read catches array)
		parameters.read(sample,{"catches"});
		// Save samples from parameters after having
		// been read
		samples.append(parameters.values());
		// Generate a random seed to be used to ensure any stochastic
		// variations is same for all procedures. Placed here so, if necessary
		// can be made constant for all replicates for testing purposes
		uint seed = vary?Uniform(0,1e10).random():10000;
		Generator.seed(seed);
		// Create a model representing current state by iterating
		// from time 0 to now...
		Model current;
		for(uint time=0;time<=time_start;time++){
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
			sum(current.biomass_spawners_unfished),
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
			for(uint time=time_start+1;time<=time_max;time++){
				//... set parameters on future model (e.g time varying parameters
				// like recruitment variation but not catches)
				parameters.set(time,future,false);
				//... operate the procedure (having 
				// procedure.operate() here, before future.update() allows 
				// for the `HistCatch` procedure which simply applies historical
				// catches
				procedures.operate(procedure,time,future);
				//... update the model
				future.update(time);
				//... track the model (for speed, only some replicates)
				if(replicate<100) tracker.get(replicate,procedure,time,future);
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

void evaluate_wrap(
	int replicates,
	std::string samples_file="feasible/output/accepted.tsv",
	uint year_start=-1
) {
	evaluate(
		replicates,
		samples_file, // samples_file
		true, // procedures_read
		-1, // procedure_select
		year_start, // year_start 
		true // vary
	);
}

void test(){
	// Read in parameters
	Parameters parameters;
	parameters.read();
	// Read in data
	Data data;
	data.read();

	for(int i=0;i<100;i++){
		Parameters parameters1 = parameters;
		Parameters parameters2 = parameters;
		
		parameters.randomise();
		auto parameter_set = parameters.vector();

		parameters1.vector(parameter_set);
		parameters2.vector(parameter_set);

		Data data1 = data;
		Data data2 = data;

		Model model1;
		Model model2;
		for(uint time=0;time<=time_now;time++){
			// Do the time step
			//... set parameters
			parameters1.set(time,model1);
			parameters2.set(time,model2);
			//... update the models
			model1.update(time);
			model2.update(time);
			//... get data
			data1.get(time,model1);
			data2.get(time,model2);
			
			/*std::cerr
					<<year(time)<<" "<<quarter(time)
					<<" "<<model1.numbers(NW,4)-model2.numbers(NW,4)
					<<std::endl;*/
		}

		if(
			model1.biomass_status()!=model2.biomass_status() or
			data1.loglike()!=data2.loglike()
		){
			std::cerr<<"Different!"<<std::endl;
		}
	}
}

template<typename Type>
Type arg(int argc, char** argv, int which, Type default_ = Type()){
	if(argc<=which) return default_;
	return boost::lexical_cast<Type>(argv[which]);
}

int main(int argc, char** argv){ 
	try {
        if(argc==1) throw std::runtime_error("No task given");
        std::string task = argv[1];
        std::cout<<"-------------"<<task<<"-------------\n"<<std::flush;
        if(task=="run") run(arg<std::string>(argc,argv,2),arg<int>(argc,argv,3),arg<int>(argc,argv,4));
		else if(task=="tracks") tracks(arg<std::string>(argc,argv,2));
        else if(task=="yield") yield();
        else if(task=="priors") priors(arg<int>(argc,argv,2));
        else if(task=="condition_feasible") condition_feasible(arg<int>(argc,argv,2));
        else if(task=="condition_ss3") condition_ss3(arg<int>(argc,argv,2));
        else if(task=="condition_demc") condition_demc(arg<int>(argc,argv,2));
        else if(task=="evaluate"){
        	evaluate(
				arg<int>(argc,argv,2,10), // int replicates=1000, 
				"feasible/output/accepted.tsv", // const std::string& samples_file="feasible/output/accepted.tsv", 
				false // bool procedures_read=true, 
				// int procedure_select=-1,
				// uint year_start=-1, 
				// bool vary=true, 
				// bool msy=true
			);
        }
        else if(task=="evaluate_wrap") evaluate_wrap(arg<int>(argc,argv,2),arg<std::string>(argc,argv,3),arg<int>(argc,argv,4));
        else if(task=="evaluate_feasible") evaluate(arg<int>(argc,argv,2),"feasible/output/accepted.tsv");
        else if(task=="evaluate_ss3") evaluate(arg<int>(argc,argv,2),"ss3/output/accepted.tsv");
        else if(task=="test") test();
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
