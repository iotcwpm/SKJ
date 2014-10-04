#pragma once

template<class Distribution>
class Variable : public Distribution, public Reflector<Variable<Distribution>> {
public:

    using Reflector<Variable<Distribution>>::derived;
    using Reflector<Variable<Distribution>>::derived_nullptr;

	Variable(void):
		value_(NAN){
	}

	Variable(const double& value):
		value_(value){
	}

	operator double(void) const {
		return value_;
	}

    bool is_na(void) const {
        return std::isnan(value_);
    }

    #define OP_(op) void operator op(const double& other){ value_ op other; }
        OP_(+=)
        OP_(-=)
        OP_(*=)
        OP_(/=)
    #undef OP_

    template<class Mirror>
    void reflect(Mirror& mirror) {
        mirror
            .data(value_,"value");
        Distribution::reflect(mirror);
    }

private:

	double value_ = NAN;
};
