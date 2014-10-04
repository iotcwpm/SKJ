#pragma once

// Ransom number generator
struct Generator : boost::mt19937 {
	Generator(void){
		seed(static_cast<unsigned int>(std::time(0)));
	}
} Generator;

/*!
A base implementation class for all probability distributions
	
All probability distributions have a member called data_ which is a boost::math distribution.
(The alternative of deriving from both boost::math distributions caused a mysterious memory bug in testing).
Random variates are produced using boost::random distributions within the random() method.
Specific classes might overide the random() method to provide greate efficiency (?) by using the boost::random distributions directly, rather than using quantile().

Boost::math defines a number of non-member properties that are common to all distributions:
	'cdf','complement','chf','hazard','kurtosis','kurtosis_excess','mean','median','mode','pdf','range','quantile','skewness','standard_deviation','support','variance'
these are wrapped into methods.
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
		return boost::math::mean(static_cast<const Derived&>(*this).boost_dist());
	}
	
	double median(void) const {
		return boost::math::median(static_cast<const Derived&>(*this).boost_dist());
	}

	double mode(void) const {
		return boost::math::mode(static_cast<const Derived&>(*this).boost_dist());
	}
	
	double sd(void) const {
		return boost::math::standard_deviation(static_cast<const Derived&>(*this).boost_dist());
	}
	
	double variance(void) const {
		return boost::math::variance(static_cast<const Derived&>(*this).boost_dist());
	}

	double skewness(void) const {
		return boost::math::skewness(static_cast<const Derived&>(*this).boost_dist());
	}
	
	double kurtosis(void) const {
		return boost::math::kurtosis(static_cast<const Derived&>(*this).boost_dist());
	}

	double kurtosis_excess(void) const {
		return boost::math::kurtosis_excess(static_cast<const Derived&>(*this).boost_dist());
	}

	bool valid(void) const {
		return true;
	}

	double likelihood(const double& x) const {
		return std::log(pdf(x));
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
    
    bool valid(void) const {
        return std::isfinite(lower) and std::isfinite(upper) and lower<upper;
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
