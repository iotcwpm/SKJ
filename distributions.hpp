#include <ctime>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/variate_generator.hpp>

/*!
A scaffolding object that holds the random number generator used in the random() methods
*/
struct Generator : boost::mt19937 {
	Generator(void){
		//! Set random number generator seed using current time
		seed(static_cast<unsigned int>(std::time(0)));
	}
} Generator;

double normal_rand(double mean, double sd) const {
    boost::normal_distribution<> distr(mean,sd);
    boost::variate_generator<boost::mt19937&,decltype(distr)> randomVariate(Generator,distr);
    return randomVariate();
}