/**
 * File for importing necessary external libraries
 */

#pragma once

// C++ standard library
#include <cmath>
#include <fstream>

// Boost library (http://www.boost.org/) for...
//... random number scaffolding...
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/variate_generator.hpp>
//... distributions
#include <boost/math/distributions/uniform.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/math/distributions/normal.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/math/distributions/lognormal.hpp>
#include <boost/random/lognormal_distribution.hpp>
//... optimisation
#include <boost/math/tools/minima.hpp>

// Stencila library (https://github.com/stencila/stencila) for ...
//... structure, array, frame and query classes
#include <stencila/structure.hpp>
#include <stencila/array.hpp>
#include <stencila/frame.hpp>
#include <stencila/query.hpp>
using namespace Stencila;
