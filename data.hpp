#pragma once

#include "common.hpp"
#include "dimensions.hpp"

namespace IOSKJ {

/**
 * Class for defining data against which the model is conditioned
 * See the `get()` method which "gets" model variables corresponding to data points at specific times.
 */
class Data /*: public Fsl::Estimation::Data<Data,Model>*/ {
public:

	// Define some aliases for convienience
	using Normal = Fsl::Math::Probability::Normal;
	using Lognormal = Fsl::Math::Probability::Lognormal;

	// Classes for each type of data...
	
	class MaldivePlCpue {
	public:
		uint year;
		uint quarter;
		float index;

		void read(std::istream& stream){
			stream>>year>>quarter>>index;
		}

		void write(std::ostream& stream){
			stream<<year<<"\t"<<quarter<<"\t"<<index;
		}
	};
	std::array<MaldivePlCpue,32> MaldivePlCpues;

	class WestPsCpue {
	public:
		uint year;
		float index;

		void read(std::istream& stream){
			stream>>year>>index;
		}

		void write(std::ostream& stream){
			stream<<year<<"\t"<<index;
		}
	};
	std::array<WestPsCpue,20> WestPsCpues;

	class SizeFrequency {
	public:
		char area;
		std::string method;
		uint year;
		uint quarter;
		std::array<float,61> props;
		unsigned int num;

		void read(std::istream& stream){
			stream>>area>>method>>year>>quarter;
			for(auto& item : props) stream>>item;
			stream>>num;
		}

		void write(std::ostream& stream){
			stream<<area<<"\t"<<method<<"\t"<<year<<"\t"<<quarter;
			for(auto& item : props) stream<<"\t"<<item;
			stream<<"\t"<<num;
		}
	};
	std::array<SizeFrequency,531> SizeFrequencies;

	class ZEstimate {
	public:
		uint year;
		uint quarter;
		float mu45;
		float mu50;
		float mu55;
		float mu60;
		float sd45;
		float sd50;
		float sd55;
		float sd60;

		void read(std::istream& stream){
			stream>>year>>quarter
				  >>mu45>>mu50>>mu55>>mu60
				  >>sd45>>sd50>>sd55>>sd60;
		}

		void write(std::ostream& stream){
			stream<<year<<"\t"<<quarter
				  <<"\t"<<mu45<<"\t"<<mu50<<"\t"<<mu55<<"\t"<<mu60
				  <<"\t"<<sd45<<"\t"<<sd50<<"\t"<<sd55<<"\t"<<sd60;
		}
	};
	std::array<ZEstimate,16> ZEstimates;

	/**
	 * Get model variables coreesponding to data at a particular time
	 */
	void get(const Model& model,uint time){

	}

	/**
	 * Calculate the log-likelihood of the fits to the data
	 */
	double likelihood(void){
		double likelihood = 0;
		return likelihood;
	}


	template<
		class Series
	>
	void read(Series& series, const std::string& filename){
		std::ifstream file(filename);
		std::string header;
		std::getline(file,header);
		for(auto& item : series) item.read(file);
	}

	template<
		class Series
	>
	void write(Series& series, const std::string& filename){
		std::ofstream file(filename);
		for(auto& item : series){
			item.write(file);
			file<<"\n";
		}
	}

	void read(void){
		read(MaldivePlCpues,"data/processed-data/m-pl-cpue.txt");
		read(WestPsCpues,"data/processed-data/w-ps-cpue.txt");
		read(SizeFrequencies,"data/processed-data/size-frequencies.txt");
		read(ZEstimates,"data/processed-data/z-estimates.txt");
	}

	void write(void){
		write(MaldivePlCpues,"output/m-pl-cpue.txt");
		write(WestPsCpues,"output/w-ps-cpue.txt");
		write(SizeFrequencies,"output/size-frequencies.txt");
		write(ZEstimates,"output/z-estimates.txt");
	}

}; // class Data

}
