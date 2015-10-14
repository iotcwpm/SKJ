#pragma once

template<class Distribution>
class Variable : public Distribution, public Structure<Variable<Distribution>> {
public:

    using Structure<Variable<Distribution>>::derived;
    using Structure<Variable<Distribution>>::derived_nullptr;

    double value = NAN;

    bool is_na(void) const {
        return std::isnan(value);
    }

	operator double(void) const {
		return value;
	}

    #define OP_(op) void operator op(const double& other){ value op other; }
        OP_(=)
        OP_(+=)
        OP_(-=)
        OP_(*=)
        OP_(/=)
    #undef OP_

    double loglike(void){
        if(not is_na() and Distribution::valid()){
            return Distribution::loglike(value);
        }
        return 0;
    }

    template<class Mirror>
    void reflect(Mirror& mirror) {
        mirror
            .data(value,"value");
        Distribution::reflect(mirror);
    }
};
