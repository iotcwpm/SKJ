#pragma once

namespace Utilities {
namespace Distributions {

/**
 * Random number generator
 */
struct Generator : boost::mt19937 {
	Generator(void){
		seed(static_cast<unsigned int>(std::time(0)));
	}
} Generator;

/**
 * Base class for all probability distributions
 */
template<
	typename Derived
>
class Distribution : public Polymorph<Derived> {
public:
	
    using Polymorph<Derived>::derived;

	void reset(void){
	}
	
	double minimum(void) const {
		return -INFINITY;
	}

	double maximum(void) const {
		return INFINITY;
	}

	double mean(void) const {
		return boost::math::mean(derived().boost_dist());
	}
	
	double median(void) const {
		return boost::math::median(derived().boost_dist());
	}

	double mode(void) const {
		return boost::math::mode(derived().boost_dist());
	}
	
	double sd(void) const {
		return boost::math::standard_deviation(derived().boost_dist());
	}
	
	double variance(void) const {
		return boost::math::variance(derived().boost_dist());
	}

	double skewness(void) const {
		return boost::math::skewness(derived().boost_dist());
	}
	
	double kurtosis(void) const {
		return boost::math::kurtosis(derived().boost_dist());
	}

	double kurtosis_excess(void) const {
		return boost::math::kurtosis_excess(derived().boost_dist());
	}

	bool valid(void) const {
		return true;
	}

	double loglike(const double& x) const {
		return std::log(derived().pdf(x));
	}
	
	double pdf(const double& x) const {
		if(derived().valid()) return boost::math::pdf(derived().boost_dist(),x);
		else return NAN;
	}
	
	double cdf(const double& x) const {
		if(derived().valid()) return boost::math::cdf(derived().boost_dist(),x);
		else return NAN;
	}
	
	double quantile(const double& p) const {
		if(derived().valid()) return boost::math::quantile(derived().boost_dist(),p);
		else return NAN;
	}
    
	double integral(const double& from,const double& to) const {
		return cdf(to)-cdf(from);
	}

    double random(void) const {
        auto boost_rand = derived().boost_rand();
        boost::variate_generator<boost::mt19937&,decltype(boost_rand)> random(Generator,boost_rand);
        return random();
    }
};


class Fixed {    
public:
    
    double value = NAN;

    bool valid(void) const {
        return std::isfinite(value);
    }

    double minimum(void) const {
        return value;
    }

    double maximum(void) const {
        return value;
    }
    
    double mean(void) const {
        return value;
    }
    
    double sd(void) const {
        return 0;
    }

    double random(void) const {
        return value;
    };

    double likelihood(const double& x) const {
        return (x==value)?0:-INFINITY;
    }

    double pdf(const double& x) const {
        return (x==value)?1:0;
    }

    double quantile(const double& p) const {
        return value;
    }

    template<class Mirror>
    void reflect(Mirror& mirror) {
        mirror
            .data(value,"value")
        ;
    }
};

class Beta : public Distribution<Beta> {
public:

    double alpha = NAN;
    double beta = NAN;

    Beta(const double& alpha = NAN, const double& beta = NAN):
        alpha(alpha),
        beta(beta){        
    }

    Beta& mean_sd(const double& mean, const double& sd){
        double var = sd*sd;
        alpha = boost::math::beta_distribution<>::find_alpha(mean,var);
        beta = boost::math::beta_distribution<>::find_beta(mean,var);
        return *this;
    }

    boost::math::beta_distribution<> boost_dist(void) const {
        return boost::math::beta_distribution<>(alpha,beta);
    }

    boost::random::beta_distribution<> boost_rand(void) const {
        return boost::random::beta_distribution<>(alpha,beta);
    }

    template<class Mirror>
    void reflect(Mirror& mirror) {
        mirror
            .data(alpha,"alpha")
            .data(beta,"beta")
        ;
    }
};


class Normal : public Distribution<Normal> {
public:

    double mean = NAN;
    double sd = NAN;

    Normal(double mean = NAN, double sd = NAN): mean(mean),sd(sd){}
    
    bool valid(void) const {
        return std::isfinite(mean) and sd>0;
    }

    boost::math::normal boost_dist(void) const {
        return boost::math::normal(mean,sd);
    }
    
    boost::normal_distribution<> boost_rand(void) const {
        return boost::normal_distribution<>(mean,sd);
    }

    template<class Mirror>
    void reflect(Mirror& mirror) {
        mirror
            .data(mean,"mean")
            .data(sd,"sd")
        ;
    }
};


class Lognormal : public Distribution<Lognormal> {
public:

    double location = NAN;
    double dispersion = NAN;

    Lognormal(double location = NAN, double dispersion = NAN): location(location),dispersion(dispersion){}

    double minimum(void) const {
        return std::numeric_limits<double>::epsilon();
    }

    bool valid(void) const {
        return location>0 and dispersion>0;
    }

    boost::math::lognormal boost_dist(void) const {
        return boost::math::lognormal(location,dispersion);
    }

    boost::lognormal_distribution<> boost_rand(void) const {
        return boost::lognormal_distribution<>(location,dispersion);
    }
    
    template<class Mirror>
    void reflect(Mirror& mirror) {
        mirror
            .data(location,"location")
            .data(dispersion,"dispersion")
        ;
    }
};


class Uniform : public Distribution<Uniform> {
public:

    double lower = NAN;
    double upper = NAN;

    Uniform(double lower = NAN,double upper = NAN):
        lower(lower),
        upper(upper){
    }
    
    bool valid(void) const {
        return std::isfinite(lower) and std::isfinite(upper) and lower<upper;
    }

    double minimum(void) const {
        return lower;
    }

    double maximum(void) const {
        return upper;
    }

    boost::math::uniform boost_dist(void) const {
        return boost::math::uniform(lower,upper);
    }

    double random(void) const {
        // If lower and upper are equal then boost random number generator
        // will loop endlessly attempting to create a valid value. So escape that condition...
        if(lower==upper) return lower;
        else{
            boost::uniform_real<> distr(lower,upper);
            boost::variate_generator<boost::mt19937&,decltype(distr)> randomVariate(Generator,distr);
            return randomVariate();
        }
    }

    template<class Mirror>
    void reflect(Mirror& mirror) {
        mirror
            .data(lower,"lower")
            .data(upper,"upper")
        ;
    }
};


class FournierRobustifiedMultivariateNormal : public Distribution<FournierRobustifiedMultivariateNormal> {
public:

    double proportion;
    double size;
    static double max_size;

    FournierRobustifiedMultivariateNormal(const double& proportion = NAN, const double& size = NAN):
        proportion(proportion),
        size(size){
    }

    bool valid(void) const {
        return std::isfinite(proportion) and size>0;
    }

    double minimum(void) const {
        return 0;
    }

    double maximum(void) const {
        return 1;
    }

    double mean(void) const {
        return proportion;
    }

    double sd(void) const {
        return proportion * (1-proportion);
    }

    double loglike(const double& x) const {
        double n_apos = std::min(size,max_size);
        double e_apos = (1-x)*x+0.1/40.0;
        return 0.5*e_apos+std::log(std::exp(-std::pow(proportion-x,2)/(2*e_apos/n_apos))+0.01);
    }

    template<class Mirror>
    void reflect(Mirror& mirror) {
        mirror
            .data(proportion,"proportion")
            .data(size,"size")
        ;
    }
};
double FournierRobustifiedMultivariateNormal::max_size = 1000;

template<typename Base>
class Truncated : public Base {
public:
    double min;
    double max;
    
    Truncated(double a = NAN,double b = NAN,double min_ = -INFINITY, double max_ = INFINITY):
        Base(a,b),min(min_),max(max_){}

    double minimum(void) const {
        return min;
    }

    double maximum(void) const {
        return max;
    }

    double random(void) const {
        double trial = Base::random();
        if(trial<min or trial>max) return random();
        else return trial;
    };

    double pdf(const double& x){
        if(x<min or x>max) return 0;
        else return Base::pdf(x);
    }

    template<class Mirror>
    void reflect(Mirror& mirror) {
        Base::reflect(mirror);
        mirror
            .data(min,"min")
            .data(max,"max")
        ;
    }
};

} // namespace Distributions
} // namespace IOSKJ
