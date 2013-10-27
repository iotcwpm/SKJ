#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/variance.hpp>

using namespace boost::accumulators;

typedef accumulator_set<
	double, 
	stats<
		tag::mean,
		tag::variance
	>
> Stats;
