#pragma once

template<class Distribution>
class Variable : public Distribution, public Structure<Variable<Distribution>> {
public:

    using Structure<Variable<Distribution>>::derived;
    using Structure<Variable<Distribution>>::derived_nullptr;

    bool is_na(void) const {
        return std::isnan(value_);
    }

	operator double(void) const {
		return value_;
	}

    #define OP_(op) void operator op(const double& other){ value_ op other; }
        OP_(=)
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
