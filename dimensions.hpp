#pragma once

#include "common.hpp"

namespace IOSKJ {

typedef unsigned int uint;

uint period;
uint year;
uint quarter;

void time(uint period){
	year = 1950 + period/4;
	quarter = period==0?0:period%4;
}

STENCILA_ARRAY_DIM(Quarter,quarters,quarter,4)

STENCILA_ARRAY_DIM(Region,regions,region,3);
STENCILA_ARRAY_DIM(RegionFrom,region_froms,region_from,3);
enum {
	W = 0,
	M = 1,
	E = 2
};

STENCILA_ARRAY_DIM(Age,ages,age,24);

STENCILA_ARRAY_DIM(Size,sizes,size,40);
STENCILA_ARRAY_DIM(SizeFrom,size_froms,size_from,40);

STENCILA_ARRAY_DIM(Method,methods,method,5);
enum {
	PS = 0,
	PL = 1,
	GN = 2,
	LI = 3,
	OT = 4
};

}
