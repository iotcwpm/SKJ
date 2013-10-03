#pragma once

#include "common.hpp"

namespace IOSKJ {

typedef unsigned int uint;

uint time = 0;
const uint times = (2013+25-1950)*4;
uint year;
uint quarter;

STENCILA_ARRAY_DIM(Region,regions,region,3);
STENCILA_ARRAY_DIM(RegionTo,region_tos,region_to,3);
enum {
	W = 0,
	M = 1,
	E = 2
};

STENCILA_ARRAY_DIM(Age,ages,age,6);

STENCILA_ARRAY_DIM(Size,sizes,size,40);

STENCILA_ARRAY_DIM(Method,methods,method,5);
enum {
	PS = 0,
	PL = 1,
	GN = 2,
	LI = 3,
	OT = 4
};

}
