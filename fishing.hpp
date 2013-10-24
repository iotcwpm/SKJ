#pragma once

#include <array>

#include "spline.hpp"

#include "common.hpp"
#include "dimensions.hpp"

namespace IOSKJ {

class Fishing {
public:

	typedef std::array<double,5> SelectivityPoints;
	Array<SelectivityPoints,Method> selectivity_points;

	Array<double,Method,Size> selectivities;

	void defaults(void){
		selectivity_points(PS)[0] = 0.0;
		selectivity_points(PS)[1] = 0.1;
		selectivity_points(PS)[2] = 0.3;
		selectivity_points(PS)[3] = 0.5;
		selectivity_points(PS)[4] = 1.0;

		selectivity_points(PL)[0] = 0.0;
		selectivity_points(PL)[1] = 0.1;
		selectivity_points(PL)[2] = 0.3;
		selectivity_points(PL)[3] = 0.5;
		selectivity_points(PL)[4] = 1.0;

		selectivity_points(GN)[0] = 0.0;
		selectivity_points(GN)[1] = 0.1;
		selectivity_points(GN)[2] = 0.3;
		selectivity_points(GN)[3] = 0.5;
		selectivity_points(GN)[4] = 1.0;

		selectivity_points(LI)[0] = 0.0;
		selectivity_points(LI)[1] = 0.1;
		selectivity_points(LI)[2] = 0.3;
		selectivity_points(LI)[3] = 0.5;
		selectivity_points(LI)[4] = 1.0;

		selectivity_points(OT)[0] = 0.0;
		selectivity_points(OT)[1] = 0.1;
		selectivity_points(OT)[2] = 0.3;
		selectivity_points(OT)[3] = 0.5;
		selectivity_points(OT)[4] = 1.0;
	}
	
	void init(void){

		for(auto method : methods){
			auto points = selectivity_points(method);
			Spline<double,double> selectivity_spline(
				{0,20,40,60,80},
				std::vector<double>(points.begin(),points.end())
			);

			for(auto size : sizes){
				selectivities(method,size) = selectivity_spline.interpolate(size*2+1);
			}
		}

		write();
	}

	void step(void){
	}

	void write(void){
		selectivities.write("output/fishing-selectivities.txt");
	}

};

}
