#pragma once

#include <string>
#include <array>
#include <fstream>

class Data {
public:

	class NominalCatch {
	public:
		char area;
		std::string method;
		unsigned short int year;
		unsigned short int quarter;
		float catches;

		void read(std::istream& stream){
			stream>>area>>method>>year>>quarter>>catches;
		}

		void write(std::ostream& stream){
			stream<<area<<"\t"<<method<<"\t"<<year<<"\t"<<quarter<<"\t"<<catches;
		}
	};
	std::array<NominalCatch,760> NominalCatches;

	class MaldivePlCpue {
	public:
		unsigned short int year;
		unsigned short int quarter;
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
		unsigned short int year;
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
		unsigned short int year;
		unsigned short int quarter;
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
		unsigned short int year;
		unsigned short int quarter;
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
		read(NominalCatches,"data/processed-data/nominal-catches.txt");
		read(MaldivePlCpues,"data/processed-data/m-pl-cpue.txt");
		read(WestPsCpues,"data/processed-data/w-ps-cpue.txt");
		read(SizeFrequencies,"data/processed-data/size-frequencies.txt");
		read(ZEstimates,"data/processed-data/z-estimates.txt");
	}

	void write(void){
		write(NominalCatches,"output/nominal-catches.txt");
		write(MaldivePlCpues,"output/m-pl-cpue.txt");
		write(WestPsCpues,"output/w-ps-cpue.txt");
		write(SizeFrequencies,"output/size-frequencies.txt");
		write(ZEstimates,"output/z-estimates.txt");
	}

} data;
