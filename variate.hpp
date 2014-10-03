#pragma once

class Variate {
public:

	Variate(void):
		value_(NAN){
	}

	Variate(const double& value):
		value_(value){
	}

	operator double(void) const {
		return value_;
	}

    #define OP_(op) void operator op(const double& other){ value_ op other; }
        OP_(+=)
        OP_(-=)
        OP_(*=)
        OP_(/=)
    #undef OP_

private:

	double value_ = NAN;
};
