#pragma once

#include "common.hpp"

namespace IOSKJ {

typedef unsigned int uint;

STENCILA_DIM(Quarter,quarters,quarter,4);

STENCILA_DIM(Region,regions,region,3);
STENCILA_DIM(RegionFrom,region_froms,region_from,3);
enum {
	W = 0,
	M = 1,
	E = 2
};

STENCILA_DIM(Age,ages,age,24);

STENCILA_DIM(Size,sizes,size,40);
STENCILA_DIM(SizeFrom,size_froms,size_from,40);

STENCILA_DIM(Method,methods,method,5);
enum {
	PS = 0,
	PL = 1,
	GN = 2,
	LI = 3,
	OT = 4
};

STENCILA_DIM(SelectivityKnot,selectivity_knots,selectivity_knot,5);

}
