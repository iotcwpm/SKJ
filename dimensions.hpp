#pragma once

#include "imports.hpp"

namespace IOSKJ {

typedef unsigned int uint;

const uint year_min = 1950;
const uint year_max = 2035;

uint time_calc(uint year,uint quarter){
	return (year-year_min)*4+quarter;
}
uint year(uint time){
	return year_min+time/4;
}
uint quarter(uint time){
	return time%4;
}

class Year : public Dimension<Year,year_max-year_min+1,year_min> {
public:
	Year(void):Dimension<Year,year_max-year_min+1,year_min>("year"){}
	static const char* name(void) { return "year"; }
} years;

STENCILA_DIM(Quarter,quarters,quarter,4);

/**
 * @name DataYear
 * 
 * A dimension for the years where there is data
 *
 * This allows grids for data to be dimensioned with a subset years
 * so that they take up less memory and are quicker to iterate over
 */
const uint data_years_size = 2014-1982+1;
class DataYear : public Dimension<DataYear,data_years_size,1982> {
public:
	DataYear(void):Dimension<DataYear,data_years_size,1982>("data_year"){}
	static const char* name(void) { return "data_year"; }
} data_years;

/**
 * @name RecdevYear
 * 
 * A dimension for the years where recruitment deviations
 * are estimated or external estimates are avaialble
 */
STENCILA_DIM_RANGE(RecdevYear,recdev_years,recdev_year,1985,2012);

STENCILA_DIM(Region,regions,region,3);
STENCILA_DIM(RegionFrom,region_froms,region_from,3);
enum {
	WE = 0,
	MA = 1,
	EA = 2
};

STENCILA_DIM(Age,ages,age,24);

STENCILA_DIM(Size,sizes,size,40);
STENCILA_DIM(SizeFrom,size_froms,size_from,40);

STENCILA_DIM(Method,methods,method,4);
enum {
	PS = 0,
	PL = 1,
	GN = 2,
	OT = 3
};

STENCILA_DIM(SelectivityKnot,selectivity_knots,selectivity_knot,7);

STENCILA_DIM(ZSize,z_sizes,z_size,4);

}
