#include <ctime>
#include <cmath>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/lognormal_distribution.hpp>

/**
 * A scaffolding object that holds the random number generator used in the random() methods
 */
struct Generator : boost::mt19937 {
	Generator(void){
		//! Set random number generator seed using current time
		seed(static_cast<unsigned int>(std::time(0)));
	}
} Generator;

/**
 * @{
 * @name Normal distribution
 */

/**
 * Generate a random number from a normal distribution
 */
double normal_rand(double mean, double sd) {
    boost::normal_distribution<> distr(mean,sd);
    boost::variate_generator<boost::mt19937&,decltype(distr)> randomVariate(Generator,distr);
    return randomVariate();
}

/**
 * Based on normal_cdf function in abmblib: http://github.com/trophia/admblib
 * @todo check 
 */
double normal_cdf(const double& x, const double& mean, const double& sd){
   	double p;
    if(x-mean==0) p = 0.5;
	else {
		double z = (x-mean)/sd;
      	double t = 1 / (1 + 0.33267 * z*z);
      	p = 0.4361836 * t - 0.120167 * std::pow(t,2) + 0.937298 * std::pow(t,3);
      	p = 1 - std::exp(-std::pow(z,2) / 2) / std::pow(2*M_PI,0.5) * p;
      	if (z < 0) p = 1 - p;
   		if (p < 0) p = 0;
	}
   	return p;
}

double normal_integral(const double& from,const double& to,const double& mean,const double& sd){
	return normal_cdf(to,mean,sd)-normal_cdf(from,mean,sd);
}

/**
 * @}
 */

/**
 * @{
 * @name Lognormal distribution
 */

/**
 * Generate a random number from a lognormal distribution
 */
double lognormal_rand(double mean, double sd) {
    boost::lognormal_distribution<> distr(mean,sd);
    boost::variate_generator<boost::mt19937&,decltype(distr)> randomVariate(Generator,distr);
    return randomVariate();
}

/**
 * @}
 */